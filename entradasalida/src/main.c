#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <IO-utils/Operations.h>

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

    //cliente se conecta al sevidor 
    int resultHandshakeKernell = connectAndHandshake(ip_kernel, puerto_kernel, IO, "kernel", logger);
    op x;
    //espera a recibir una instrucción y la ejecuta
    while (recv(resultHandshakeKernell,&x,sizeof(op),MSG_WAITALL)){
        switch(x.op_code){
            case IO_GEN_SLEEP:
            log_info(logger,"Operacion: <IO_GEN_SLEEP>");
            sleep(tiempo_unidad_trabajo/1000 * x.tiempo);
            enviar_mensaje("1",resultHandshakeKernell);
            default:
            log_info(logger,"Operacion: <NO DEFINIDA>");
        }
    }
    return 0;
}
