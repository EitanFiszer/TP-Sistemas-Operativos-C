#include "main.h"

int retardo_respuesta = 0;
char* path_instrucciones = "";
int TAM_PAGINA = 0;
int TAM_MEMORIA = 0;
int server_fd = 0;

int socketCpu;
int socketKernel;

sem_t sem_cpu;
sem_t sem_kernel;

void liberarMemoria() {
    log_destroy(logger);
    config_destroy(config);
}

Handshake esperar_cliente_memoria(int socket_servidor, t_log* logger) {
    int socket_cliente;
    socket_cliente = accept(socket_servidor, NULL, NULL);

    uint32_t handshake;
    uint32_t resultOk = 0;
    uint32_t resultError = -1;
    Handshake handshakeCliente;

    recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);

    if (socket_cliente == -1) {
        log_error(logger, "Error al aceptar un nuevo cliente");
        send(socket_cliente, &resultError, sizeof(uint32_t), 0);
    } else if(handshake == CPU) {
        log_info(logger, "Se conectó un CPU!");
        send(socket_cliente, &TAM_PAGINA, sizeof(uint32_t), 0);
    } else {
        log_error(logger, "Se conectó un cliente");
        send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
    }

    handshakeCliente.modulo = handshake;
    handshakeCliente.socket = socket_cliente;

    return handshakeCliente;
}

void iniciarSemaforos() {
    sem_init(&sem_cpu, 0, 0);
    sem_init(&sem_kernel, 0, 0);
}

int main(int argc, char* argv[]) {
    logger = iniciar_logger("memoria.log", "Memoria");
    config = iniciar_config("memoria.config");

    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
    TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");

    char* string = NULL;
    bitarray = iniciarBitarray(string);

    server_fd = iniciar_servidor(puerto_escucha, logger);

    iniciarSemaforos();

    log_info(logger, "[MEMORIA] Escuchando en el puerto: %s", puerto_escucha);

    inicializarMemoria();

    iniciarHilos();

    bool seConectoKernel = false;
    bool seConectoCpu = false;


    while(!seConectoKernel || !seConectoCpu) {
        Handshake res = esperar_cliente_memoria(server_fd, logger);
        int cliente = res.socket;
        ID modulo = res.modulo;
        
        if (modulo == KERNEL) {
            if (seConectoKernel) {
              log_error(logger, "Ya se conectó un Kernel");
              break;
            }
            log_info(logger, "Se conectó un Kernel en el socket %d", cliente);
            socketKernel = cliente;
            sem_post(&sem_kernel);
            seConectoKernel = true;
        } else if (modulo == CPU) {
            if (seConectoCpu) {
              log_error(logger, "Ya se conectó un CPU");
              break;
            }
            log_info(logger, "Se conectó un CPU en el socket %d", cliente);
            socketCpu = cliente;
            sem_post(&sem_cpu);
            seConectoCpu = true;
        } else if (modulo == IO) {
            log_info(logger, "Se conectó un IO");
        } else {
            log_error(logger, "Se conectó un cliente desconocido");
            break;
        }
    }

    liberarMemoria();

    return EXIT_SUCCESS;
}


	// while(!seConectoKernel || !seConectoCpu){
	// 	Handshake res = esperar_cliente(server_fd, logger);
	// 	int modulo = res.modulo;
	// 	int socket_cliente = res.socket;
	// 	log_info(logger, "SOCKET: %d", socket_cliente);
	// 	switch (modulo) {
	// 		case CPU:
	// 			log_info(logger, "Se conecto un CPU");
	// 			break;
	// 		case KERNEL:
	// 			log_info(logger, "Se conecto un Kernel");
	// 			break;
	// 		case IO:
	// 			log_info(logger, "Se conecto un IO");
	// 			break;
	// 		default:
	// 			log_error(logger, "Se conecto un cliente desconocido");
	// 			break;
	// 	}
	// }