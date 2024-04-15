#include <stdlib.h>
#include <stdio.h>
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    // decir_hola("Kernel");

    t_log* logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("kernel.config");

    int puerto = config_get_int_value(config, "PUERTO_ESCUCHA");

    char* stringParaLogger = string_from_format("[KERNEL] Escuchando en el puerto: %d", puerto);
    log_info(logger, stringParaLogger);

    // leemos las configs
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* ip_cpu = config_get_string_value(config, "IP_CPU");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    char* puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    char* puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");

    // creamos el servidor
    int server_fd = iniciar_servidor(puerto_escucha, logger);

    //cliente se conecta al sevidor 
   // int resultHandshake = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
   // int resultHandshake = connectAndHandshake(ip_cpu, puerto_cpu_interrupt, KERNEL, "cpu", logger);

    int resultHandshake = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);


    return 0;
}