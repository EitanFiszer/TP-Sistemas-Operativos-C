#include <semaphore.h>
#include "hilos.h"
#include <commons/config.h>
#include <unistd.h>
#include <utils/constants.h>
#include <utils/client.h>
struct args {
    char* nombre;
    char* path_config;
};

extern t_log* logger;

int conexionKernell (char* ip, char* puerto, char* tipo_interfaz, char* nombre){
    int resultHandshake = connectAndHandshake(ip, puerto, IO, "kernel", logger); 

    t_paquete* paq = crear_paquete();
    t_paquete_entre* paquete=malloc(sizeof(t_paquete_entre));  
    t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));

    payload->tipo_interfaz = tipo_interfaz;
    payload->nombre = nombre;
    paquete->payload = payload;
    paquete->operacion = IO_INTERFAZ_CREADA;

    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paquete,resultHandshake);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A KERNEL");
    eliminar_paquete(paq);
    free(paquete);

    return resultHandshake;
}

int conexionMemoria (char* ip, char* puerto){
    int resultHandshake = connectAndHandshake(ip, puerto, IO, "memoria", logger); 
    return resultHandshake;
}


void hilo_generica(void *argumentos) {
    struct args *nombreYpath  = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

    t_config* config = config_create(path_config);
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);

    while(1){
        t_list* paq = recibir_paquete(resultHandshakeKernell);
        t_paquete_entre* paquete_dispatch = list_get(paq, 0);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        if (op != INSTRUCCION_IO) {
            log_error(logger, "Operacion no soportada");
            break;
        }

        op_io* operacionRecibida = paquete_dispatch->payload;

        switch(operacionRecibida->op_code){
            case IO_GEN:
                tiempo_gen=(int)operacionRecibida->tiempo;
                log_info(logger,"Operacion: <IO_GEN_SLEEP>");
                sleep(tiempo_unidad_trabajo / 1000 * tiempo_gen);
            break;
            default:
            log_info(logger,"Operacion: <NO DEFINIDA>");
        }
    }
}


void hilo_stdin(void* argumentos){
    struct args *nombreYpath  = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

     t_config* config = config_create(path_config);

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);


     while(1){
        t_list* paq = recibir_paquete(resultHandshakeKernell);
        t_paquete_entre* paquete_dispatch = list_get(paq, 0);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        if (op != INSTRUCCION_IO) {
            log_error(logger, "Operacion no soportada");
            break;
        }

        op_io* operacionRecibida = paquete_dispatch->payload;

        switch(operacionRecibida->op_code){
            case IO_STDIN:
                int resultHandshakeMemoria = conexionMemoria(ip_memoria, puerto_memoria);
            break;
            default:
            log_info(logger,"Operacion: <NO DEFINIDA>");
        }
    }
}


void hilo_stdout(void* argumentos){
    struct args *nombreYpath  = argumentos;
    char* path_config = nombreYpath->path_config;
    char* nombre = nombreYpath->nombre;

     t_config* config = config_create(path_config);

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    int resultHandshakeKernell = conexionKernell(ip_kernel, puerto_kernel, tipo_interfaz, nombre);


     while(1){
        t_list* paq = recibir_paquete(resultHandshakeKernell);
        t_paquete_entre* paquete_dispatch = list_get(paq, 0);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        if (op != INSTRUCCION_IO) {
            log_error(logger, "Operacion no soportada");
            break;
        }

        op_io* operacionRecibida = paquete_dispatch->payload;

        switch(operacionRecibida->op_code){
            case IO_STDIN:
                int resultHandshakeMemoria = conexionMemoria(ip_memoria, puerto_memoria);
            break;
            default:
            log_info(logger,"Operacion: <NO DEFINIDA>");
        }
    }
}
