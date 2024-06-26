#include "hilos.h"

#include <commons/config.h>
#include <semaphore.h>
#include <unistd.h>
#include <utils/client.h>
#include <utils/constants.h>
#include <utils/envios.h>
#include <utils/serializacion.h>
#include <utils/server.h>

struct args {
    char* nombre;
    char* path_config;
};

extern t_log* logger;

int conexionKernell(char* ip, char* puerto, char* tipo_interfaz, char* nombre) {
    int resultHandshake = connectAndHandshake(ip, puerto, IO, "kernel", logger);

    t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));

    payload->tipo_interfaz = tipo_interfaz;
    payload->nombre = nombre;

    int size_pay;
    void* buffer = serializar_interfaz_creada(payload, &size_pay);
    enviar_paquete_entre(resultHandshake, IO_INTERFAZ_CREADA, buffer, size_pay);

    // t_paquete* paq = crear_paquete();
    // t_paquete_entre* paquete=malloc(sizeof(t_paquete_entre));
    // paquete->operacion = IO_INTERFAZ_CREADA;
    // paquete->size_payload = size_pay;
    // paquete->payload = buffer;

    // agregar_paquete_entre_a_paquete(paq,paquete);
    // enviar_paquete(paquete,resultHandshake);

    log_info(logger, "PAQUETE CREADO Y ENVIADO A KERNEL");

    return resultHandshake;
}

int conexionMemoria(char* ip, char* puerto) {
    int resultHandshake = connectAndHandshake(ip, puerto, IO, "memoria", logger);
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
                log_info(logger, "Operacion: <IO_GEN_SLEEP>");
                sleep(tiempo_unidad_trabajo / 1000 * tiempo_gen);
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
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(resultHandshakeKernell);
        OP_CODES_ENTRE op = paquete_entre->operacion;

        switch (op) {
            case IO_STDIN_READ:
                t_payload_io_stdin_read_de_kernel_a_io* operacionRecibida = deserializar_io_stdin_read_de_kernel_a_io(paquete_dispatch->payload);
                int resultHandshakeMemoria = conexionMemoria(ip_memoria, puerto_memoria);

                if (resultHandshakeMemoria < 0) {
                    log_error(logger, "Error conectando a memoria");
                    break;
                }

                char input[operacionRecibida->tamaño];
                printf("Ingrese texto: ");
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;  // Elimina el salto de línea

                t_payload_escribir_memoria* payload = malloc(sizeof(t_payload_escribir_memoria));
                payload->direccion = operacionRecibida->direccion;
                payload->cadena = input;
                payload->size_cadena = strlen(input) + 1;  // +1 para incluir el '\0'

                int size_payload;
                void* payloadSerializado = serializar_escribir_memoria(payload, &size_payload);
                enviar_paquete_entre(resultHandshakeMemoria, ESCRIBIR_MEMORIA, payloadSerializado, sizeof(t_payload_escribir_memoria));

                // Cerrar conexión con memoria
                close(resultHandshakeMemoria);
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
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);

    while (1) {
        // t_list* paq = recibir_paquete(resultHandshakeKernell);
        t_paquete_entre* paquete_dispatch = recibir_paquete_entre(resultHandshakeKernell);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        // AGREGAR DESERIALIZACIÓN Y SERIALIZACIÓN
        switch (op) {
            case IO_STDOUT_WRITE:
                t_payload_io_stdout_write* operacionRecibida = paquete_dispatch->payload;
                int resultHandshakeMemoria = conexionMemoria(ip_memoria, puerto_memoria);
                
                if (resultHandshakeMemoria < 0) {
                    log_error(logger, "Error conectando a memoria");
                    break;
                }

                // Supongamos que el payload contiene la dirección de memoria a leer
                uint32_t direccion_memoria = operacionRecibida->direccion;

                // CAMBIAR A enviar_paquete_entre
                // Enviar solicitud de lectura de memoria
                t_paquete* paquete = crear_paquete();
                agregar_a_paquete(paquete, &direccion_memoria, sizeof(direccion_memoria));
                enviar_paquete(paquete, resultHandshakeMemoria);
                eliminar_paquete(paquete);

                // Recibir respuesta de la memoria
                t_paquete_entre* respuesta = recibir_paquete_entre(resultHandshakeMemoria);
                char* valor_leido = (char*)respuesta->payload;

                log_info(logger, "Valor leído de memoria: %s", valor_leido);

                // Mostrar el valor leído en la consola
                printf("Valor leído de memoria en la dirección %u: %s\n", direccion_memoria, valor_leido);

                // Limpiar la lista recibida y cerrar conexión
                list_destroy(respuesta);
                close(resultHandshakeMemoria);

                break;
            default:
                log_info(logger, "Operacion: <NO DEFINIDA>");
        }
    }
}
