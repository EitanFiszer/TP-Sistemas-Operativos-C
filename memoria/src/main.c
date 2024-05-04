#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <memoria-utils/procesos.h>


int main(int argc, char* argv[]) {
    t_log* logger = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("memoria.config");

    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	int retardo = config_get_int_value(config, "RETARDO_RESPUESTA");

    int server_fd = iniciar_servidor(puerto_escucha, logger);

    char* stringParaLogger = string_from_format("[MEMORIA] Escuchando en el puerto: %s", puerto_escucha);
	log_info(logger, stringParaLogger);
	
	while(1){
		handshake_t res = esperar_cliente(server_fd, logger);
		int modulo = res.modulo;
		int socket_cliente = res.socket;

		switch (modulo) {
			case CPU:
				log_info(logger, "Se conecto un CPU!");

				// char* archivo = res.archivo;
				// sleep(retardo);
				break;
			case KERNEL:
				log_info(logger, "Se conecto un Kernel");

				int sig_id = 0;
				proceso_t* proceso = crear_proceso(sig_id++, logger);
				
				break;
			case IO:
				log_info(logger, "Se conecto un IO");
				break;
			default:
				log_error(logger, "Se conecto un cliente desconocido");
				break;
		}
	}
    

    return 0;
}
