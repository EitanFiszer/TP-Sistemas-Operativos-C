#include "espera.h"

void *esperar_paquetes_memoria(void *arg)
{

    int *socket = (int *)arg;
    int socketMemoria = *socket;

    while (1)
    {
        t_paquete_entre *unPaquete = recibir_paquete_entre(socketMemoria);
        switch (unPaquete->operacion)
        {
        case INSTRUCCIONES_CARGADAS:
            int *PID = (int *)unPaquete->payload;
            cargar_ready_por_pid(*PID);
            break;
        default:
            log_error(logger, "no se recibio paquete de la memoria, error");
            break;
        }
    }
}

void *esperar_paquetes_cpu_dispatch(void *arg)
{
    int *socket_dispatch = (int *)arg;
    while (1)
    {

        t_paquete_entre *paquete_dispatch = recibir_paquete_entre(*socket_dispatch);
        switch (paquete_dispatch->operacion)
        {
        case INTERRUMPIO_PROCESO:
            int *PID = (int *)paquete_dispatch->payload;
            cargar_ready_por_pid(*PID);
            break;

        case SYSCALL:
            desalojar();
            atender_syscall(paquete_dispatch->payload); // aca modifico el quantum
            // puede ser instruccion de IO
            break;

        case WAIT:
            t_payload_wait *paquete_wait = (t_payload_wait *)paquete_dispatch->payload;
            atender_wait(paquete_wait->pcb, paquete_wait->recurso);
            break;

        case SIGNAL:
            t_payload_signal *paquete_signal = (t_payload_signal *)paquete_dispatch->payload;
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



void enviar_instrucciones_memoria(char *path, int PID)
{
    t_paquete *nuevo_paquete = crear_paquete();
    t_paquete_entre *instruccion;
    instruccion = malloc(sizeof(t_paquete_entre));
    instruccion->operacion = CREAR_PROCESO;
    instruccion->size_payload=sizeof(payload_crear_proceso);
    payload_crear_proceso *payload = malloc(sizeof(payload_crear_proceso));
    payload->path = path;
    payload->pid = PID;
    instruccion->payload = payload;
   
   agregar_paquete_entre_a_paquete(nuevo_paquete,instruccion);
    // envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
    enviar_paquete(nuevo_paquete, resultHandshakeMemoria);
    eliminar_paquete(nuevo_paquete);
    free(instruccion);
}

void enviar_paquete_memoria(OP_CODES_ENTRE operacion, void *payload, int size_payload)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload=size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq,paquete);
    enviar_paquete(paq, resultHandshakeMemoria);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A MEMORIA");
    eliminar_paquete(paq);
    free(paquete);
}

void enviar_paquete_cpu_dispatch(OP_CODES_ENTRE operacion, void *payload, int size_payload)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload=size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq,paquete);
    enviar_paquete(paq, resultHandshakeDispatch);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A CPU DISPATCH");
    eliminar_paquete(paq);
    free(paquete);
}

void interrumpir()
{
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