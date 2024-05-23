#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <utils/client.c>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
//#include <IO-utils/Operations.h>
#include <utils/constants.h>

int main(int argc, char* argv[]) {
    t_log* logger = log_create("entradasalida.log", "Entrada_Salida", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("entradasalida.config");

     // leemos las configs
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //int resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, IO, "memoria", logger);

    payload_intruccion_io x = malloc(sizeof(payload_intruccion_io));

    //Se conecta a kernel y le envia las interfaces
    int resultHandshakeKernell = connectAndHandshake(ip_kernel, puerto_kernel, IO, "kernel", logger); 
    t_paquete *paq = crear_paquete();
    t_paquete_entre* paquete=malloc(sizeof(t_paquete_entre));  
    t_payload_io_gen_sleep* payload = malloc(sizeof(t_payload_io_gen_sleep));
    
    payload->interfaz = x.op;
    paquete->payload = payload;

    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paquete,resultHandshakeKernell)
    log_info(logger, "PAQUETE CREADO Y ENVIADO A KERNEL");
    eliminar_paquete(paq);
    free(paquete);
    
    //espera a recibir una instrucción y la ejecuta
    while(1){
        t_list* paq = rebicir_paquete(resultHandshakeKernell);
        t_paquete_entre* paquete_dispatch = list_get(paquete, 0);
        OP_CODES_ENTRE op = paquete_dispatch->operacion;

        if (op !== INSTRUCCION_IO) {
            log_error(logger, "Operacion no soportada");
            break;
        }

        op_io operacionRecibida = paquete_dispatch->payload;

        switch(operacionRecibida.op_code){
            case IO_GEN:
                char* interfaz = operacionRecibida.interfaz;
                int tiempo = operacionRecibida.tiempo;
                
                log_info(logger,"Operacion: <IO_GEN_SLEEP>");
                sleep(tiempo_unidad_trabajo / 1000 * tiempo);
                enviar_mensaje("1",resultHandshakeKernell);
            case IO_STDIN:
            case IO_STDOUT:
            case IO_FS:
            default:
            log_info(logger,"Operacion: <NO DEFINIDA>");
        }
    }
    return 0;
}
