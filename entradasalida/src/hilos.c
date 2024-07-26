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

struct args {
    char* nombre;
    char* path_config;
};

extern t_log* logger;
extern int socketMemoria;

void crearArchivodebloques(int block_count, int block_size, char* pathbase) {
    size_t tamano_total = block_size * block_count;

    char* filepath;
    snprintf(filepath, sizeof(filepath), "%s/bloques.dat", pathbase);

    FILE *archivo = fopen("bloques.dat", "wb");
    if (!archivo) {
        perror("Error al abrir el archivo de bloques");
        exit(EXIT_FAILURE);
    }

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
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);

    while (1) {
        t_paquete_entre* paquete_entre = recibir_paquete_entre(resultHandshakeKernell);
        OP_CODES_ENTRE op = paquete_entre->operacion;


        switch (op) {
            case IO_STDIN_READ:

                t_payload_io_stdin_read_de_kernel_a_io* operacionRecibida = deserializar_io_stdin_read_de_kernel_a_io(paquete_entre->payload);

                int inputSize = operacionRecibida->tam;
                char* input;
                printf("Ingrese texto: ");
                fgets(input, inputSize, stdin);
                input[strcspn(input, "\n")] = 0;  // Elimina el salto de línea
                
                t_payload_escribir_memoria* payload = malloc(sizeof(t_payload_escribir_memoria));
                payload->direccion = operacionRecibida->direccionFisica;
                payload->cadena = input;
                payload->size_cadena = strlen(input) + 1;  // +1 para incluir el '\0'

                int size_payload;
                void* payloadSerializado = serializar_escribir_memoria(payload, &size_payload);
                enviar_paquete_entre(socketMemoria, ESCRIBIR_MEMORIA, payloadSerializado, sizeof(t_payload_escribir_memoria));

            
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

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);

    while (1) {
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(resultHandshakeKernell);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        // AGREGAR DESERIALIZACIÓN Y SERIALIZACIÓN
        switch (op) {
            case IO_STDOUT_WRITE:
                t_payload_io_stdout_write* operacionRecibida = deserializar_io_stdout_write(paquete_dispatch->payload);

                t_payload_solicitar_dato_memoria* payloadMandar = malloc(sizeof(t_payload_solicitar_dato_memoria));
                payloadMandar->direccion = operacionRecibida->direccionFisica;

                enviar_paquete_entre(socketMemoria, SOLICITAR_DATO_MEMORIA, payloadMandar, sizeof(t_payload_solicitar_dato_memoria));

                t_paquete_entre* respuesta = recibir_paquete_entre(socketMemoria);
                char* valor_leido = (char*)respuesta->payload;

                log_info(logger, "Valor leído de memoria: %s", valor_leido);

                printf("Valor leído de memoria en la dirección %u: %s\n", operacionRecibida->direccionFisica, valor_leido);

                list_destroy(respuesta);

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
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    int block_count = config_get_int_value(config, "BLOCK_COUNT");
    char* path_base = config_get_string_value(config, "PATH_BASE_DIALFS");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);



    /*
    char cwd[256];
    getcwd(cwd, sizeof(cwd));

    char* path_archivo = malloc(strlen(cwd) + strlen(path_instrucciones) + strlen(nombre_archivo) + 1);
    strcpy(path_archivo, cwd);
    strcat(path_archivo, path_instrucciones);
    strcat(path_archivo, nombre_archivo);
    */


    crearArchivodebloques(block_count, block_size, path_base);
    crear_bitmap(block_count, path_base);

    while (1) {
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(resultHandshakeKernell);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;
        
        switch(op) {
            case IO_FS_CREATE:
                crear_archivo();
                    
            break;

            case IO_FS_DELETE:
            break;

            case IO_FS_TRUNCATE:
            break;

            case IO_FS_WRITE:
            break;

            case IO_FS_READ:
            break;

        }
    }
}
