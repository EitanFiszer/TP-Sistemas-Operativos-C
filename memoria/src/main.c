#include "main.h"

#include <memoria-utils/memoria.h>
#include <utils/serializacion.h>

int retardo_respuesta = 0;
char* path_instrucciones = "";
int TAM_PAGINA = 0;
int TAM_MEMORIA = 0;
int server_fd = 0;

int socketCpu;
int socketKernel;
int socketIO;

Memoria memoria;
pthread_mutex_t mutexMemoria;

void signal_callback_handler(int signum) {
    log_info(logger, "Finalizando por CTRL+C, op %d\n", signum);
    liberarMemoria();
}

void liberarMemoria() {
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(server_fd);
    free(memoria.memoria);
    exit(1);
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
    } else if (handshake == CPU) {
        log_info(logger, "Se conectó un CPU!");
        send(socket_cliente, &TAM_PAGINA, sizeof(uint32_t), 0);
    } else if (handshake == KERNEL) {
        log_info(logger, "Se conectó un Kernel!");
        send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
    } else if (handshake == IO) {
        log_info(logger, "Se conectó un IO!");
        send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
    } else {
        log_error(logger, "Se conectó un cliente desconocido");
        send(socket_cliente, &resultError, sizeof(uint32_t), 0);
    }

    handshakeCliente.modulo = handshake;
    handshakeCliente.socket = socket_cliente;

    return handshakeCliente;
}

void esperarConexiones() {
    while (1) {
        Handshake res = esperar_cliente_memoria(server_fd, logger);
        int modulo = res.modulo;
        int *cliente = malloc(sizeof(int));
        *cliente = res.socket;

        switch (modulo) {
            case CPU:
                log_info(logger, "Se conectó un CPU en el socket %d", *cliente);
                pthread_t hiloEsperaCpu;
                socketCpu = *cliente;
                pthread_create(&hiloEsperaCpu, NULL, (void*)esperar_paquetes_cpu, NULL);   
                pthread_detach(hiloEsperaCpu);
                break;
            case KERNEL:
                log_info(logger, "Se conectó un Kernel en el socket %d", *cliente);
                pthread_t hiloEsperaKernel;
                socketKernel = *cliente;
                pthread_create(&hiloEsperaKernel, NULL, (void*)esperar_paquetes_kernel, NULL);
                pthread_detach(hiloEsperaKernel);
                break;
            case IO:
                log_info(logger, "Se conectó un IO en el socket %d", *cliente);
                pthread_t hiloEsperaIO;
                pthread_create(&hiloEsperaIO, NULL, (void*)atender_cliente_io, cliente);
                pthread_detach(hiloEsperaIO);
                break;
            default:
                // log_info(logger, "Operación desconocida de KERNEL");
                break;
        }
    }

    printf("Memoria inicializada, esperando hilos\n");
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_callback_handler);

    logger = iniciar_logger("memoria.log", "Memoria");
    config = iniciar_config("memoria.config");

    char* puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");
    path_instrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
    TAM_PAGINA = config_get_int_value(config, "TAM_PAGINA");
    TAM_MEMORIA = config_get_int_value(config, "TAM_MEMORIA");

    char* string = NULL;
    marcosLibres = iniciarBitarray(string);
    server_fd = iniciar_servidor(puerto_escucha, logger);
    inicializarMemoria();

    log_info(logger, "[MEMORIA] Escuchando en el puerto: %s", puerto_escucha);
    esperarConexiones();
    // probarTodo();

    liberarMemoria();

    return EXIT_SUCCESS;
}