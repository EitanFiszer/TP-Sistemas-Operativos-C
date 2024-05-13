#include <commons/log.h>
#include <commons/string.h>

struct args {
    char* puerto;
    t_log* logger;
};

void conexion_interrupt(void *argumentos) {
    /*
    struct args *puertoYLogger = argumentos;
    char* puerto = puertoYLogger->puerto;
    t_log* logger = puertoYLogger->logger;
    // creamos el servidor
    int server_interrupt_fd = iniciar_servidor(puerto, logger);
    char* stringParaLogger = string_from_format("[CPU] Escuchando en el puerto interrupt: %s", puerto);
    log_info(logger, stringParaLogger);


    handshake_t res = esperar_cliente(server_interrupt_fd, logger);
    int modulo = res.modulo;
    int socket_cliente = res.socket;
    switch (modulo) {
	    case KERNEL:
		    log_info(logger, "Se conecto un Kernel");
    	break;
        default:
		    log_error(logger, "Se conecto un cliente desconocido");
		break;
    }
    */
}