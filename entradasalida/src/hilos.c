#include "hilos.h"
#include <commons/config.h>
#include <semaphore.h>
#include <unistd.h>
#include <utils/client.h>
#include <utils/constants.h>
#include <utils/envios.h>
#include <utils/serializacion.h>
#include <utils/server.h>
#include <readline/readline.h>
#include "operacionesFS.h"
#include "bitmap.h"
#include "utils.h"


struct args {
    char* nombre;
    char* path_config;
};

int block_count;
int block_size;


extern t_log* logger;
extern char* ip_kernel;
extern char* ip_memoria;
extern char* puerto_kernel;
extern char* puerto_memoria;
char* path_base_fs;

int conexionKernell(char* puerto, char* tipo_interfaz, char* nombre) {
    int resultHandshake = connectAndHandshake(ip_kernel, puerto_kernel, IO, "kernel", logger);

    t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));

    payload->tipo_interfaz = tipo_interfaz;
    payload->nombre = nombre;

    int size_pay;
    void* buffer = serializar_interfaz_creada(payload, &size_pay);
    enviar_paquete_entre(resultHandshake, IO_INTERFAZ_CREADA, buffer, size_pay);
    log_info(logger, "Interfaz %s conectada a kernel", nombre);
    return resultHandshake;
}

int conexionMemoria(char* puerto, char* tipo_interfaz, char* nombre) {
    int resultHandshake = connectAndHandshake(ip_memoria, puerto_memoria, IO, "memoria", logger);
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
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(puerto_kernel, tipo_interfaz, nombre);

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
                int pid = operacionRecibida->pcb->PID;
                int tiempo_gen = operacionRecibida->tiempo;
                log_info(logger, "PID: %d - Operacion: <IO_GEN_SLEEP>",pid);
                sleep(tiempo_unidad_trabajo / 1000 * tiempo_gen);
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

    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int socketKernell = conexionKernell(puerto_kernel, tipo_interfaz, nombre);
    int socketMemoria = conexionMemoria(puerto_memoria, tipo_interfaz, nombre);


    while (1) {
        t_paquete_entre* paquete_entre = recibir_paquete_entre(socketKernell);
        if (paquete_entre == NULL) {
            log_error(logger, "Error al recibir paquete, finalizando hilo");
            return;
        }

        OP_CODES_ENTRE op = paquete_entre->operacion;

        switch (op) {
            case IO_STDIN_READ:

                t_payload_io_stdin_read* operacionRecibida = deserializar_io_stdin_read(paquete_entre->payload);

                int inputSize = operacionRecibida->tam;
                int pid = operacionRecibida->pcb->PID;

                log_info(logger, "PID: %d - Operacion: <IO_STDIN_READ>",pid);

                printf("Ingrese texto: \n");
                char *fullInput = readline(">");
                char *input = malloc(inputSize);
                strncpy(input, fullInput, inputSize);
                
                t_payload_escribir_memoria* payload = malloc(sizeof(t_payload_escribir_memoria));
                payload->direccion = operacionRecibida->dirFisica;
                payload->dato = (void*)input;
                payload->size_cadena = strlen(input) + 1;  // +1 para incluir el '\0'

                int size_payload;
                void* payloadSerializado = serializar_escribir_memoria(payload, &size_payload);
                enviar_paquete_entre(socketMemoria, ESCRIBIR_MEMORIA, payloadSerializado, size_payload);
                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0);


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

    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int socketKernell = conexionKernell(puerto_kernel, tipo_interfaz, nombre);
    int socketMemoria = conexionMemoria(puerto_memoria, tipo_interfaz, nombre);

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

                int pid= operacionRecibida->pcb->PID;
                log_info(logger, "PID: %d - Operacion: <IO_STDOUT_WRITE>",pid);
                
                t_payload_solicitar_dato_memoria* payloadMandar = malloc(sizeof(t_payload_solicitar_dato_memoria));
                payloadMandar->direccion = operacionRecibida->direccionFisica;
                payloadMandar->tam=operacionRecibida->tam;
            

                enviar_paquete_entre(socketMemoria, SOLICITAR_DATO_MEMORIA, payloadMandar, sizeof(t_payload_solicitar_dato_memoria));

                t_paquete_entre* respuesta = recibir_paquete_entre(socketMemoria);
                void* dato = respuesta->payload;
                

                log_info(logger, "Valor leído de memoria: %s", (char*)dato);
                printf("Valor leído de memoria en la dirección %u: %s\n", operacionRecibida->direccionFisica, (char*)dato);

                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0);

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
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    int retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");

    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    path_base_fs = config_get_string_value(config, "PATH_BASE_DIALFS");

    int socketKernell = conexionKernell(puerto_kernel, tipo_interfaz, nombre);
    int socketMemoria = conexionMemoria(puerto_memoria, tipo_interfaz, nombre);
    
    inicializar_FS();

    while (1) {
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(socketKernell);
        if (paquete_dispatch == NULL) {
            log_error(logger, "Error al recibir paquete, finalizando hilo");
            return;
        }

        OP_CODES_ENTRE op = paquete_dispatch->operacion;
        
        switch(op) {
            case IO_FS_CREATE:
                sleep(tiempo_unidad_trabajo/1000);
                t_payload_fs_create* payloadcreate=deserializar_fs_create(paquete_dispatch->payload);
                int pid_create = payloadcreate->pcb->PID;
                log_info(logger, "PID: %d - Crear Archivo: %s",pid_create,payloadcreate->nombreArchivo);
                crear_archivo(payloadcreate->nombreArchivo);   
                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0); 
            break;
            case IO_FS_DELETE:
                sleep(tiempo_unidad_trabajo/1000);
                t_payload_fs_create* payloaddelete=deserializar_fs_create(paquete_dispatch->payload);
                int pid_delete = payloaddelete->pcb->PID;
                log_info(logger, "PID: %d - Eliminar Archivo: %s",pid_delete,payloadcreate->nombreArchivo);
                delete_archivo(payloaddelete->nombreArchivo);
                enviar_paquete_entre(socketKernell, TERMINE_OPERACION, NULL, 0);

            break;

            case IO_FS_TRUNCATE:
                sleep(tiempo_unidad_trabajo/1000);
                t_payload_fs_truncate* payloadtruncate=deserializar_fs_truncate(paquete_dispatch->payload);
                int pid_truncate=payloadtruncate->pcb->PID;
                log_info(logger, "PID: %d - Truncar Archivo: %s - Tamaño: %d",pid_truncate,payloadtruncate->nombreArchivo, payloadtruncate->tam);
                truncate_archivo(payloadtruncate->nombreArchivo, payloadtruncate->tam, retraso_compactacion);
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
