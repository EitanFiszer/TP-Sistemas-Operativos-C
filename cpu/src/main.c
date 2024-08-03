#include "main.h"

#include <signal.h>
#include <utils/iniciar.h>

registros_t registros;
int socketKernel;
int socketMemoria;
int TAM_PAGINA;
t_log* logger;
t_config* config;
int interrupcion;

int server_dispatch_fd;
int server_interrupt_fd;

t_list* TLB;
int TLB_MAX_SIZE;
char* TLB_ALGORITMO_REEMPLAZO;

pthread_mutex_t mutex_interrupcion;

struct args {
    char* puerto;
    t_log* logger;
};

void signal_callback_handler(int signum) {
    log_info(logger, "Finalizando por CTRL+C, op %d\n", signum);
    finalizarCPU();
}

void finalizarCPU() {
    // printf("Finalizando CPU\n");
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(server_dispatch_fd);
    liberar_conexion(server_interrupt_fd);
    // liberar_conexion(socketKernel);
    exit(1);
}

void conexion_interrupt(void* argumentos) {
    struct args* puertoYLogger = argumentos;
    char* puerto = puertoYLogger->puerto;
    t_log* logger = puertoYLogger->logger;
    // creamos el servidor
    server_interrupt_fd = iniciar_servidor(puerto, logger);
    log_info(logger, "[CPU] Escuchando en el puerto interrupt: %s", puerto);

    Handshake res = esperar_cliente(server_interrupt_fd, logger);
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

    while (1) {
        t_paquete_entre* paquete = recibir_paquete_entre(socket_cliente);
        if (paquete == NULL) {
            log_error(logger, "Error al recibir paquete de kernel interrupt, finalizando conexiones");
            finalizarCPU();
        }
        switch (paquete->operacion) {
            case INTERRUMPIR_PROCESO:
                // t_PCB* pcb = (t_PCB*)paquete->payload;
                pthread_mutex_lock(&mutex_interrupcion);
                interrupcion = true;
                pthread_mutex_unlock(&mutex_interrupcion);
                // log_info(logger, "Interrupcion recibida para PID %d", pcb->PID);
                break;
            default:
                log_error(logger, "Operacion desconocida interrupt %d", paquete->operacion);
                break;
        }
    }
}

int getHayInterrupcion() {
    pthread_mutex_lock(&mutex_interrupcion);
    int hayInterrupcion = interrupcion;
    pthread_mutex_unlock(&mutex_interrupcion);
    return hayInterrupcion;
}

// TESTS
void testConnMem() {
    OP_CODES_ENTRE op = PC_A_INSTRUCCION;
    t_payload_pc_a_instruccion* payload = malloc(sizeof(t_payload_pc_a_instruccion));
    payload->PID = 6;
    payload->program_counter = 9;
    t_paquete* paq = crear_paquete();
    t_paquete_entre* paq_entre = malloc(sizeof(t_paquete_entre));

    paq_entre->operacion = op;
    paq_entre->size_payload = sizeof(t_payload_pc_a_instruccion);
    paq_entre->payload = payload;

    agregar_paquete_entre_a_paquete(paq, paq_entre);
    enviar_paquete(paq, socketMemoria);
    eliminar_paquete(paq);
    free(payload);
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signal_callback_handler); //mata todo cpu, no anda muy bien

    // creamos logs y configs
    logger = iniciar_logger("cpu.log", "CPU");
    config = iniciar_config("cpu.config");

    // leemos las configs
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    char* puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    TLB_MAX_SIZE = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    TLB_ALGORITMO_REEMPLAZO = config_get_string_value(config, "ALGORITMO_TLB");

    log_debug(logger, "Configuraciones leidas %s, %s, %s, %s", ip_memoria, puerto_memoria, puerto_escucha_dispatch, puerto_escucha_interrupt);

    pthread_mutex_init(&mutex_interrupcion, NULL);

    // declaramos los hilos
    pthread_t hilo_interrupt;
    args argumentos_interrupt;
    argumentos_interrupt.puerto = puerto_escucha_interrupt;
    argumentos_interrupt.logger = logger;

    pthread_create(&hilo_interrupt, NULL, (void*)conexion_interrupt, (void*)&argumentos_interrupt);
    pthread_detach(hilo_interrupt);

    // inicializamos la TLB
    TLB = list_create();

    // El cliente se conecta
    handshake_cpu_memoria handshakeMemoria = handshake_memoria(ip_memoria, puerto_memoria);
    socketMemoria = handshakeMemoria.socket;
    TAM_PAGINA = handshakeMemoria.tam_pagina;
    if (socketMemoria == -1) {
        log_error(logger, "No se pudo conectar con la memoria");
        finalizarCPU();
    }
    // printf("Handshake socket: %d, TAM_PAG: %d\n", socketMemoria, TAM_PAGINA);

    // El kernel se conecta a nosotros (CPU) y recibimos su handshake para poder recibir el pcb de parte del kernel
    server_dispatch_fd = iniciar_servidor(puerto_escucha_dispatch, logger);
    log_info(logger, "CPU listo para recibir al cliente");
    socketKernel = -1;
    while (socketKernel == -1) {
        Handshake handshake_res = esperar_cliente(server_dispatch_fd, logger);
        if (handshake_res.modulo == KERNEL) {
            socketKernel = handshake_res.socket;
            log_info(logger, "Se conecto un Kernel");
            break;
        } else {
            log_error(logger, "Se conecto un socket desconocido");
        }
    }

    while (1) {
        // Recibo el paquete del kernel
        t_paquete_entre* paq = recibir_paquete_entre(socketKernel);
        if (paq == NULL) {
            log_error(logger, "No se pudo recibir el paquete kernel");
            finalizarCPU();
        }
        t_PCB* pcb = (t_PCB*)paq->payload;
        bool terminoProceso = false;
        switch (paq->operacion) {
            case EXEC_PROCESO:
                while (1) {
                    registros = pcb->cpu_registro;
                    char* instruccionRecibida;
                    int ok = fetchInstruccion(pcb, socketMemoria, &instruccionRecibida, logger);
                    if (ok == -1) {
                        free(instruccionRecibida);
                        log_error(logger, "PROCESO TERMINÓ EJECUCIÓN: PID %d", pcb->PID);
                        terminoProceso = true;
                        OP_CODES_ENTRE op = TERMINO_EJECUCION;
                        // Devolver el PCB al kernel
                        enviar_pcb_kernel(pcb, op);
                        break;
                    }
                    terminoProceso = false;

                    // Decodifico la instruccion en opcode y parametros
                    instruccionCPU_t* instruccion = dividirInstruccion(instruccionRecibida);
                    free(instruccionRecibida);

                    // Ejecuto la instruccion
                    ejecutarInstruccion(instruccion, pcb, logger, socketKernel);
                    if(getHayInterrupcion()) break;
                }
                if (getHayInterrupcion() && !terminoProceso) {
                    log_info(logger, "Interrupcion alcanzada");

                    pthread_mutex_lock(&mutex_interrupcion);
                    interrupcion = false;
                    pthread_mutex_unlock(&mutex_interrupcion);
                    // Interrumpir el proceso
                    enviar_pcb_kernel(pcb, INTERRUMPIO_PROCESO);
                    break;
                }
                break;
            default:
                log_error(logger, "Operacion desconocida");
                break;
        }
    }

    finalizarCPU();
}