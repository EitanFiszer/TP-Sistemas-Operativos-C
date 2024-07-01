#include "espera.h"

void atender_cliente(void *socket) {
    int *socket_cliente_IO = (int *)socket;
    while (1) {
        t_paquete_entre *unPaquete = recibir_paquete_entre(socket_cliente_IO);
        if (unPaquete == NULL) {
            log_error(logger, "Cliente IO desconectado, socket: %d", socket_cliente_IO);

        } else {
            switch (unPaquete->operacion) {
                case IO_INTERFAZ_CREADA:
                    t_payload_interfaz_creada *datos_interfaz = deserializar_interfaz_creada(unPaquete->payload);
                    agregar_interfaz(datos_interfaz->nombre, datos_interfaz->tipo_interfaz, socket_cliente_IO);
                    log_info(logger, "NUEVA INTERFAZ %s CONECTADA", datos_interfaz->nombre);
                    break;
                default:
                    log_error(logger, "no se recibio paquete de la memoria, error");
                    break;
            }
        }
    }
}


void *esperar_paquetes_memoria(void *arg) {
    // int *socket = (int *)arg;
    // int socketMemoria = *socket;

    while (1) {
        t_paquete_entre *unPaquete = recibir_paquete_entre(resultHandshakeMemoria);
        if (unPaquete == NULL) {
            log_error(logger,"Hubo un error al recibir paquete de Memoria, cerrando kernel");
            finalizar_kernel();
        } else {
            switch (unPaquete->operacion) {
                case INSTRUCCIONES_CARGADAS:
                    int *PID = (int *)unPaquete->payload;
                    log_info(logger, "INSTRUCCIONES CARGADAS del PID: %d", *PID);
                    cargar_ready_por_pid(*PID);
                    break;
                default:
                    log_error(logger, "no se recibio paquete de la memoria, error");
                    break;
            }
        }
    }
}

void *esperar_paquetes_cpu_dispatch(void *arg) {
    // int *socket_dispatch = (int *)arg;
    while (1) {
        t_paquete_entre *paquete_dispatch = recibir_paquete_entre(resultHandshakeDispatch);
        switch (paquete_dispatch->operacion) {
            case INTERRUMPIO_PROCESO:
                t_PCB *PCB = (t_PCB *)paquete_dispatch->payload;
                desalojar();
                cargar_ready(PCB);
                break;

            case SYSCALL:
                desalojar();
                atender_syscall(paquete_dispatch->payload);  // aca modifico el quantum
                // puede ser instruccion de IO
                break;

            case WAIT:
                t_payload_wait_signal *paquete_wait = deserializar_wait_signal(paquete_dispatch->payload);
                atender_wait(paquete_wait->pcb, paquete_wait->recurso);
                break;

            case SIGNAL:
                t_payload_wait_signal *paquete_signal = deserializar_wait_signal(paquete_dispatch->payload);
                atender_signal(paquete_signal->pcb, paquete_signal->recurso);
                break;
            case TERMINO_EJECUCION:
                desalojar();
                t_PCB *pcb_dispatch = (t_PCB *)paquete_dispatch->payload;
                log_info(logger,"Finaliza el proceso %d - Motivo: SUCCESS", pcb_dispatch->PID);
                lts_ex(pcb_dispatch);
                /// PROCESO TERMINADO SE DESALOJA Y SE ENVIA A EXIT
                break;
            case IO_GEN_SLEEP:
                break;
            case IO_FS_TRUNCATE:
                break;
            case IO_FS_WRITE:
                break;
            case IO_FS_READ:
                break;
            case IO_FS_CREATE:
                break;
            case IO_FS_DELETE:
                break;
            default:
                log_error(logger, "no se recibio paquete de la memoria, error");
                break;
        }
    }
    return NULL;
}

void enviar_instrucciones_memoria(char *path, int PID) {
    printf("enviando instrucciones a memoria, path: %s, PID: %d\n", path, PID);
    t_payload_crear_proceso *payload = malloc(sizeof(t_payload_crear_proceso));
    payload->path = path;
    payload->pid = PID;
    int size_crear;

    void *buffer = serializar_crear_proceso(payload, &size_crear);
    enviar_paquete_entre(resultHandshakeMemoria, CREAR_PROCESO, buffer, size_crear);
}

void enviar_paquete_memoria(OP_CODES_ENTRE operacion, void *payload, int size_payload) {
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, resultHandshakeMemoria);
    eliminar_paquete(paq);
    free(paquete);
}

void enviar_paquete_cpu_dispatch(OP_CODES_ENTRE operacion, void *payload, int size_payload) {
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, resultHandshakeDispatch);
    eliminar_paquete(paq);
    free(paquete);
}

void interrumpir() {
    t_paquete *paquete_fin_de_q = crear_paquete();
    t_paquete_entre *fin_q = malloc(sizeof(t_paquete_entre));
    fin_q->operacion = INTERRUMPIR_PROCESO;
    fin_q->size_payload = sizeof(int);
    int instruccion_valida = 1;
    fin_q->payload = &instruccion_valida;
    agregar_paquete_entre_a_paquete(paquete_fin_de_q, fin_q);
    enviar_paquete(paquete_fin_de_q, resultHandshakeInterrupt);
    eliminar_paquete(paquete_fin_de_q);
    free(fin_q);
    log_info(logger, "Se interrumpio el proceso");
}
void finalizar_kernel() {
    printf("Finalizando KERNEL\n");
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(resultHandshakeDispatch);
    liberar_conexion(resultHandshakeInterrupt);
    liberar_conexion(resultHandshakeMemoria);
    exit(1);
}