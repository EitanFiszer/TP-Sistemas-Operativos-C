#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    // creamos logs y configs
    t_log* logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("cpu.config");

    // leemos las configs
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    // creamos el servidor
    int server_interrupt_fd = iniciar_servidor(puerto_escucha_interrupt, logger);
    char* stringParaLogger = string_from_format("[CPU] Escuchando en el puerto interrupt: %s", puerto_escucha_interrupt);
    log_info(logger, stringParaLogger);
    int server_dispatch_fd = iniciar_servidor(puerto_escucha_dispatch, logger);
    stringParaLogger = string_from_format("[CPU] Escuchando en el puerto dispatch: %s", puerto_escucha_dispatch);
    log_info(logger, stringParaLogger);

    // esperar_cliente(server_interrupt_fd, logger);
    // esperar_cliente(server_dispatch_fd, logger);

    //cliente se conecta al sevidor 
    int resultHandshake = connectAndHandshake(ip_memoria, puerto_memoria, CPU, "memoria", logger);
    printf("Handshake socket: %d\n", resultHandshake);



    return 0;
}