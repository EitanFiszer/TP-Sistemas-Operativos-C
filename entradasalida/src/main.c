#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    t_log* logger = log_create("entradasalida.log", "Entrada_Salida", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("entradasalida.config");

    int puerto = config_get_int_value(config, "PUERTO_MEMORIA");

    char* stringParaLogger = string_from_format("[EyS] Escuchando en el puerto: %d", puerto);
    log_info(logger, stringParaLogger);

     // leemos las configs
    char* ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    //cliente se conecta al sevidor 
    int resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, IO, "memoria", logger);
    int resultHandshakeKernell = connectAndHandshake(ip_kernel, puerto_kernel, IO, "kernel", logger);

    return 0;
}
