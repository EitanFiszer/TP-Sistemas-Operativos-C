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
	char* path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");

    int server_fd = iniciar_servidor(puerto_escucha, logger);

    char* stringParaLogger = string_from_format("[MEMORIA] Escuchando en el puerto: %s", puerto_escucha);
	log_info(logger, stringParaLogger);
	const char* nombre_archivo = "test.txt";

	inicializarMemoria(logger);

	while(1){
		
		//KERNEL
		int sig_id = 0;
		crearProceso(path_instrucciones, sig_id++, logger);
		log_info(logger, obtenerInstruccion(0, 1));
		finalizarProceso(sig_id-1, logger);

		handshake_t res = esperar_cliente(server_fd, logger);
		int modulo = res.modulo;
		int socket_cliente = res.socket;

		switch (modulo) {
			case CPU:
				log_info(logger, "Se conecto un CPU!");
				//La CPU solicitará a la memoria las instrucciones cargadas en la Memoria de Instrucciones, mediante solicitudes por medio del Program Counter.

				sleep(retardo);
				break;
			case KERNEL:
				//Evaluar si se trata de la creación o finalización de un proceso.
				//Si se está creando un proceso, se debe recibir el nombre del archivo que contiene las instrucciones.
				log_info(logger, "Se conecto un Kernel");

				// int sig_id = 0;
				// proceso_t* proceso = crear_proceso(sig_id++, logger);
				
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
