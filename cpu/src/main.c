#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <cpu-utils/registros.h>
#include <cpu-utils/conexiones.h>
#include <pthread.h>


int main(int argc, char* argv[]) {
    // creamos logs y configs
    t_log* logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("cpu.config");

    // leemos las configs
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    char* puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    // declaramos los hilos
    pthread_t hilo_dispatch;
    pthread_t hilo_interrupt;

    args argumentos_dispatch;
    argumentos_dispatch.puerto = puerto_escucha_dispatch;
    argumentos_dispatch.logger = logger;

    args argumentos_interrupt;
    argumentos_interrupt.puerto = puerto_escucha_interrupt;
    argumentos_interrupt.logger = logger;
    
    pthread_create(&hilo_dispatch, NULL, conexion_dispatch, (void*)&argumentos_dispatch);
    pthread_create(&hilo_interrupt, NULL, conexion_dispatch, (void*)&argumentos_interrupt);

    //VER HILOS PARA CONECTAR DOS PUERTOS
    /*int server_dispatch_fd = iniciar_servidor(puerto_escucha_dispatch, logger);
    stringParaLogger = string_from_format("[CPU] Escuchando en el puerto dispatch: %s", puerto_escucha_dispatch);
    log_info(logger, stringParaLogger);*/

    //EL cliente se conecta 
    int resultHandshake = connectAndHandshake(ip_memoria, puerto_memoria, CPU, "memoria", logger);
    printf("Handshake socket: %d\n", resultHandshake);

    pthread_detach(hilo_dispatch);
    pthread_join(hilo_interrupt, NULL);

    return 0;
}