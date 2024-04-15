#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    t_log* logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("memoria.config");

    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    int server_fd = iniciar_servidor(puerto_escucha, logger);

    char* stringParaLogger = string_from_format("[MEMORIA] Escuchando en el puerto: %s", puerto_escucha);
	log_info(logger, stringParaLogger);
	
    Handshake res = esperar_cliente(server_fd, logger);
    int modulo = res.modulo;
    int socket_cliente = res.socket;

    switch (modulo) {
		case CPU:
			log_info(logger, "Se conecto un CPU");
			break;
		case KERNEL:
			log_info(logger, "Se conecto un Kernel");
			break;
		case IO:
			log_info(logger, "Se conecto un IO");
			break;
		default:
			log_error(logger, "Se conecto un cliente desconocido");
			break;
	}
    

    return 0;
}
