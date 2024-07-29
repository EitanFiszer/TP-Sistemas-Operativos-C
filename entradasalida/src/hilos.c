#include "hilos.h"
#include <commons/config.h>
#include <semaphore.h>
#include <unistd.h>
#include <utils/client.h>
#include <utils/constants.h>
#include <utils/envios.h>
#include <utils/serializacion.h>
#include <utils/server.h>
#include "operacionesFS.h"
#include "bitmap.h"
#include "utils.h"

int block_size;
int block_count;
char* path_base_fs;

struct args {
    char* nombre;
    char* path_config;
};

extern t_log* logger;
extern int socketMemoria;

void crearArchivodebloques(int block_count, int block_size, char* pathbase) {
    size_t tamano_total = block_size * block_count;

    FILE* archivo = crear_archivo_fs("bloques.dat");

    // Establecer el tamaño del archivo
    if (fseek(archivo, tamano_total - 1, SEEK_SET) != 0) {
        perror("Error al ajustar el tamaño del archivo");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Escribir un byte nulo al final para establecer el tamaño
    if (fwrite("", 1, 1, archivo) != 1) {
        perror("Error al escribir el byte final");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);
}



int conexionKernell(char* ip, char* puerto, char* tipo_interfaz, char* nombre) {
    int resultHandshake = connectAndHandshake(ip, puerto, IO, "kernel", logger);

    t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));

    payload->tipo_interfaz = tipo_interfaz;
    payload->nombre = nombre;

    int size_pay;
    void* buffer = serializar_interfaz_creada(payload, &size_pay);
    enviar_paquete_entre(resultHandshake, IO_INTERFAZ_CREADA, buffer, size_pay);
    log_info(logger, "Interfaz %s conectada a kernel", nombre);
    return resultHandshake;
}

int conexionMemoria(char* ip, char* puerto, char* tipo_interfaz, char* nombre) {
    int resultHandshake = connectAndHandshake(ip, puerto, IO, "memoria", logger);

    t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));

    payload->tipo_interfaz = tipo_interfaz;
    payload->nombre = nombre;

    int size_pay;
    void* buffer = serializar_interfaz_creada(payload, &size_pay);
    enviar_paquete_entre(resultHandshake, IO_INTERFAZ_CREADA, buffer, size_pay);
    log_info(logger, "Interfaz %s conectada a memoria", nombre);
    return resultHandshake;
}

void hilo_generica(void* argumentos) {
    struct args* nombreYpath = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

    t_config* config = config_create(path_config);
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);

    while (1) {
        t_paquete_entre* paquete_entre = recibir_paquete_entre(resultHandshakeKernell);
        if (paquete_entre == NULL) {
            log_error(logger, "Error al recibir paquete, finalizando hilo");
            return;
        }

        OP_CODES_ENTRE op = paquete_entre->operacion;

        switch (op) {
            case IO_GEN_SLEEP:
                t_payload_io_gen_sleep* operacionRecibida = deserializar_io_gen_sleep(paquete_entre->payload);
                int tiempo_gen = operacionRecibida->tiempo;
                log_info(logger, "Operacion: <IO_GEN_SLEEP> - PID: %d, INTERFAZ: %s, TIEMPO %d", operacionRecibida->pcb->PID, operacionRecibida->interfaz, tiempo_gen);
                sleep(tiempo_unidad_trabajo / 1000 * tiempo_gen);
                log_info(logger, "Operacion: <IO_GEN_SLEEP> - PID: %d, INTERFAZ: %s, TIEMPO %d - FINALIZADA", operacionRecibida->pcb->PID, operacionRecibida->interfaz, tiempo_gen);
                enviar_paquete_entre(resultHandshakeKernell, TERMINE_OPERACION, NULL, 0);
                break;
            default:
                log_info(logger, "Operacion: <NO DEFINIDA>");
        }
    }
}

void hilo_stdin(void* argumentos) {
    struct args* nombreYpath = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

    t_config* config = config_create(path_config);

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* ip_memoria = onfig_get_string_value(config, "IP_MEMORIA");

    int socketKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);
    int socketMemoria = conexionMemoria(ip_memoria, puerto_memoria, tipo_interfaz, nombre);


    while (1) {
        t_paquete_entre* paquete_entre = recibir_paquete_entre(socketKernell);
        if (paquete_entre == NULL) {
            log_error(logger, "Error al recibir paquete, finalizando hilo");
            return;
        }

        OP_CODES_ENTRE op = paquete_entre->operacion;


        switch (op) {
            case IO_STDIN_READ:

                t_payload_io_stdin_read_de_kernel_a_io* operacionRecibida = deserializar_io_stdin_read_de_kernel_a_io(paquete_entre->payload);

                int inputSize = operacionRecibida->tam;
                char* input;
                log_info(logger,"Ingrese texto: ");
                fgets(input, inputSize,stdin);
                
                t_payload_escribir_memoria* payload = malloc(sizeof(t_payload_escribir_memoria));
                payload->direccion = operacionRecibida->direccionFisica;
                payload->cadena = input;
                payload->size_cadena = strlen(input) + 1;  // +1 para incluir el '\0'

                int size_payload;
                void* payloadSerializado = serializar_escribir_memoria(payload, &size_payload);
                enviar_paquete_entre(socketMemoria, ESCRIBIR_MEMORIA, payloadSerializado, size_payload);

                enviar_paquete_entre(socketMemoria, TERMINE_OPERACION, NULL, 0);


                break;
            default:
                log_info(logger, "Operacion: <NO DEFINIDA>");
        }
    }
}

void hilo_stdout(void* argumentos) {
    struct args* nombreYpath = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

    t_config* config = config_create(path_config);

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* ip_memoria = onfig_get_string_value(config, "IP_MEMORIA");

    int socketKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);
    int socketMemoria = conexionMemoria(ip_memoria, puerto_memoria, tipo_interfaz, nombre);

    while (1) {
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(socketKernell);
        if (paquete_dispatch == NULL) {
            log_error(logger, "Error al recibir paquete, finalizando hilo");
            return;
        }

        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        switch (op) {
            case IO_STDOUT_WRITE:
                t_payload_io_stdout_write* operacionRecibida = deserializar_io_stdout_write(paquete_dispatch->payload);

                t_payload_solicitar_dato_memoria* payloadMandar = malloc(sizeof(t_payload_solicitar_dato_memoria));
                payloadMandar->direccion = operacionRecibida->direccionFisica;
                payloadMandar->tam=operacionRecibida->tam;

                enviar_paquete_entre(socketMemoria, SOLICITAR_DATO_MEMORIA, payloadMandar, sizeof(t_payload_solicitar_dato_memoria));

                t_paquete_entre* respuesta = recibir_paquete_entre(socketMemoria);
                void* dato = respuesta->payload;
                

                log_info(logger, "Valor leído de memoria: %s", (char*)dato);
                printf("Valor leído de memoria en la dirección %u: %s\n", operacionRecibida->direccionFisica, (char*)dato);

                enviar_paquete_entre(conexionKernell, TERMINE_OPERACION, NULL, 0);

                break;
            default:
                log_info(logger, "Operacion: <NO DEFINIDA>");
        }
    }
}

void hilo_dialfs(void* argumentos){
    struct args* nombreYpath = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

    t_config* config = config_create(path_config);

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* ip_memoria = config_get_string_value(config, "IP_KERNEL");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    int retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");

    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    path_base_fs = config_get_string_value(config, "PATH_BASE_DIALFS");

    int socketKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);
    int socketMemoria = conexionMemoria(ip_memoria, puerto_memoria, tipo_interfaz, nombre);
    crearArchivodebloques(block_count, block_size, path_base_fs);
    crear_bitmap();

    while (1) {
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(conexionKernell);
        if (paquete_dispatch == NULL) {
            log_error(logger, "Error al recibir paquete, finalizando hilo");
            return;
        }

        OP_CODES_ENTRE op = paquete_dispatch->operacion;
        
        switch(op) {
            case IO_FS_CREATE:
                sleep(tiempo_unidad_trabajo/1000);
                t_payload_fs_create* payloadcreate=deserializar_fs_create(paquete_dispatch->payload);
                crear_archivo(payloadcreate->nombreArchivo);   
                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0); 
            break;
            case IO_FS_DELETE:
                sleep(tiempo_unidad_trabajo/1000);
                t_payload_fs_create* payloaddelete=deserializar_fs_create(paquete_dispatch->payload);
                delete_archivo(payloaddelete->nombreArchivo);
                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0);

            break;

            case IO_FS_TRUNCATE:
                sleep(tiempo_unidad_trabajo/1000);
                t_payload_fs_truncate* payloadtruncate=deserializar_fs_truncate(paquete_dispatch->payload);
                truncate_archivo(payloadtruncate->nombreArchivo, payloadtruncate->regTam);
                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0);
            break;

            case IO_FS_WRITE:
            break;

            case IO_FS_READ:
            break;

            default:
                log_error(logger, "Operacion: <NO DEFINIDA>");
        }
    }
}
