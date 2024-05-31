#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <memoria-utils/procesos.h>
#include "./memoria-utils/conexiones.h"

int main(int argc, char* argv[]) {
    t_log* logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("memoria.config");

    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    int retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    char* path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");

    int server_fd = iniciar_servidor(puerto_escucha, logger);

    char* stringParaLogger = string_from_format("[MEMORIA] Escuchando en el puerto: %s", puerto_escucha);
    log_info(logger, stringParaLogger);

    inicializarMemoria(logger);

    while(1) {
        handshake_t res = esperar_cliente(server_fd, logger);
        int cliente = res.socket;
        int modulo = res.modulo;
        
        if (modulo == KERNEL) {
            log_info(logger, "Se conectó un Kernel");
            esperar_paquetes_kernel(cliente, logger, path_instrucciones);
        } else if (modulo == CPU) {
            log_info(logger, "Se conectó un CPU");
            // Acá pide instrucciones en base a PID y n de instrucción
			esperar_paquetes_cpu(cliente, logger);
        } else if (modulo == IO) {
            log_info(logger, "Se conectó un IO");
        } else {
            log_error(logger, "Se conectó un cliente desconocido");
        }

        close(cliente);
    }

    return 0;
}
