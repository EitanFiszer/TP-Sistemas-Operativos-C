#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <utils/client.c>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/constants.h>
#include <string.h>
#include <IO-utils/Definitions.h>
#include <pthread.h>
#include "hilos.h"

/*void iniciarIntGen(int resultHandshakeKernell){
    t_config* config = config_create("IntGen.config");

     // leemos las configs

    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");


    t_paquete* paq = crear_paquete();
    t_paquete_entre* paquete=malloc(sizeof(t_paquete_entre));  
    t_payload_io_gen_sleep* payload = malloc(sizeof(t_payload_io_gen_sleep));

    payload->interfaz = tipo_interfaz;
    paquete->payload = payload;

    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paquete,resultHandshakeKernell);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A KERNEL");
    eliminar_paquete(paq);
    free(paquete);
}

*/




t_interfaz* crearHilo(char* nombre, char* path_config){
    t_config* config = config_create(path_config);
    
    
    tipo_interfaz interfaz = (tipo_interfaz)config_get_string_value(config, "TIPO_INTERFAZ");
    
    void* funcion;

    switch(interfaz){
        case GENERICA:
            funcion=hilo_generica;
        break;
        case STDIN:
        break;
        case STDOUT:
        break;
        case DIALFS:
        break;
    }
     pthread_t hilo;
 
    pthread_create(&hilo, NULL, funcion, path_config);
    pthread_detach(hilo);
    
    t_interfaz* inter = malloc(sizeof(t_interfaz));
    inter->nombre = nombre;
    inter->tipo = interfaz;

    return inter;
}

int main(int argc, char* argv[]) {
    t_log* logger = log_create("entradasalida.log", "Entrada_Salida", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("EntradaSalida.config");

    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    
    int resultHandshakeKernell = connectAndHandshake(ip_kernel, puerto_kernel, IO, "kernel", logger); 
    
    t_interfaz* interfazGen = crearHilo("int1", "IntGen.config");


    //espera a recibir una instrucción y la ejecuta
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
                sem_post(&semGen);
                sem_wait(&semGenLog);
                log_info(logger,"Operacion: <IO_GEN_SLEEP>");
            break;
            case IO_STDIN:
            break;
            case IO_STDOUT:
            break;
            case IO_FS:
            break;
            default:
            
            log_info(logger,"Operacion: <NO DEFINIDA>");
        }
    }
    return 0;
}
