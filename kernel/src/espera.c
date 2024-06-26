#include "espera.h"

void atender_cliente(void *socket) {
    int *socket_cliente_IO = (int *)socket;
<<<<<<< HEAD
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
=======
    while (1)
    {
        t_paquete_entre *unPaquete = recibir_paquete_entre(*socket_cliente_IO);
        if (unPaquete == NULL)
        {
            log_error(logger, "Cliente IO desconectado, socket: %d", *socket_cliente_IO);
        }
        else
        {
            switch (unPaquete->operacion)
            {
            case IO_INTERFAZ_CREADA:
                t_payload_interfaz_creada *datos_interfaz = deserializar_interfaz_creada(unPaquete->payload);
                agregar_interfaz(datos_interfaz->nombre, datos_interfaz->tipo_interfaz, *socket_cliente_IO);
                log_info(logger, "NUEVA INTERFAZ %s CONECTADA", datos_interfaz->nombre);
                break;
            default:
                log_error(logger, "no se recibio paquete de la memoria, error");
                break;
>>>>>>> c96341f660d61b700138a1e02f9b54f64dc1de17
            }
        }
    }
}

// t_paquete* paq = crear_paquete();
//     t_paquete_entre* paquete=malloc(sizeof(t_paquete_entre));
//     t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));

//     paquete->operacion = IO_INTERFAZ_CREADA;
//     paquete->size_payload = sizeof(t_payload_interfaz_creada);

//     payload->tipo_interfaz = tipo_interfaz;
//     payload->nombre = nombre;
//     paquete->payload = payload;

void *esperar_paquetes_memoria(void *arg) {
    // int *socket = (int *)arg;
    // int socketMemoria = *socket;

    while (1) {
        t_paquete_entre *unPaquete = recibir_paquete_entre(resultHandshakeMemoria);
<<<<<<< HEAD
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
=======
        if (unPaquete == NULL)
        {
            log_error(logger, "Memoria desconectada");
        }
        else
        {
            switch (unPaquete->operacion)
            {
            case INSTRUCCIONES_CARGADAS:
                int *PID = (int *)unPaquete->payload;
                log_info(logger, "INSTRUCCIONES CARGADAS del PID: %d", *PID);
                cargar_ready_por_pid(*PID);
                break;
            default:
                log_error(logger, "no se recibio paquete de la memoria, error");
                break;
>>>>>>> c96341f660d61b700138a1e02f9b54f64dc1de17
            }
        }
    }
}

void *esperar_paquetes_cpu_dispatch(void *arg) {
    // int *socket_dispatch = (int *)arg;
    while (1) {
        t_paquete_entre *paquete_dispatch = recibir_paquete_entre(resultHandshakeDispatch);
<<<<<<< HEAD
        switch (paquete_dispatch->operacion) {
            case INTERRUMPIO_PROCESO:
                int *PID = (int *)paquete_dispatch->payload;
                cargar_ready_por_pid(*PID);
                break;
=======
        switch (paquete_dispatch->operacion)
        {
        case INTERRUMPIO_PROCESO:

            // DEPENDE PORQUE SE INTERRUMPIO EL PROCESO PUDO HABER SIDO DESALOJADO O ELIMINADO ARREGLARLO CON UN BOOL O ALGO DE ESO
            int *PID = (int *)paquete_dispatch->payload;
            cargar_ready_por_pid(*PID);
            break;
>>>>>>> c96341f660d61b700138a1e02f9b54f64dc1de17

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
                lts_ex(pcb_dispatch);
                /// PROCESO TERMINADO SE DESALOJA Y SE ENVIA A EXIT
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

    t_paquete *nuevo_paquete = crear_paquete();
    t_paquete_entre *instruccion = malloc(sizeof(t_paquete_entre));

    t_payload_crear_proceso *payload = malloc(sizeof(t_payload_crear_proceso));
    payload->path = path;
    payload->pid = PID;
    int size_crear;

    void *buffer = serializar_crear_proceso(payload, &size_crear);

    instruccion->operacion = CREAR_PROCESO;
    instruccion->size_payload = size_crear;
    instruccion->payload = buffer;

    agregar_paquete_entre_a_paquete(nuevo_paquete, instruccion);

    // envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
    enviar_paquete(nuevo_paquete, resultHandshakeMemoria);
    eliminar_paquete(nuevo_paquete);
    free(payload);
    free(instruccion);
}

void enviar_paquete_memoria(OP_CODES_ENTRE operacion, void *payload, int size_payload) {
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, resultHandshakeMemoria);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A MEMORIA");
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
    log_info(logger, "PAQUETE CREADO Y ENVIADO A CPU DISPATCH");
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