#include "espera.h"

void esperar_paquetes_memoria(int socketMemoria)
{
    while (1)
    {
        t_list *paquete = recibir_paquete(socketMemoria);
        t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
        switch (paquete_dispatch->operacion)
        {
        case INSTRUCCIONES_CARGADAS:
            int PID = paquete_dispatch->payload;
            cargar_ready_por_pid(PID);
            break;
        default:
            log_error(logger, "no se recibio paquete de la memoria, error");
            break;
        }
    }
}

void esperar_paquetes_cpu_dispatch(int socketDispatch)
{
    while (1)
    {
        t_list *paquete = recibir_paquete(socketDispatch);
        t_paquete_entre *paquete_dispatch = list_get(paquete, 0);

        t_PCB* pcb_dispatch;

        switch (paquete_dispatch->operacion)
        {
        case INTERRUMPIO_PROCESO:
            desalojar();
            pcb_dispatch = paquete_dispatch->payload;
            cargar_ready(pcb_dispatch);
            log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: READY", pcb_dispatch->PID);
            //ESTO ES PORQUE KERNEL PIDIO QUE SE INTERRUMPA POR FIN DE QUANTUM PROBABLEMENTE
            //ENTONCES VUELVO A CARGAR EL PROCESO EN COLA DE READY AL FINAL
            break;
        case SYSCALL:
            desalojar();
            atender_syscall(paquete_dispatch->payload); //aca modifico el quantum 
            //puede ser instruccion de IO 
            break;
        case WAIT:
            t_payload_wait* paquete_wait = paquete_dispatch->payload;
            atender_wait(paquete_wait->pcb,paquete_wait->recurso);
            break;
        
        case SIGNAL:
            t_payload_signal* paquete_signal = paquete_dispatch->payload;
            atender_signal(paquete_signal->pcb,paquete_signal->recurso);
            break;
        case TERMINO_EJECUCION:
            desalojar();
            pcb_dispatch = paquete_dispatch->payload;
            lts_ex(pcb_dispatch);
            ///PROCESO TERMINADO SE DESALOJA Y SE ENVIA A EXIT
            break;
        default:
            log_error(logger, "no se recibio paquete de la CPU, error");
            break;
        }
    }
}

void enviar_instrucciones_memoria(char* path, int PID,int socketMemoria)
{
    t_paquete *nuevo_paquete = crear_paquete();
    t_paquete_entre *instruccion;
    instruccion = malloc(sizeof(t_paquete_entre));
    instruccion->operacion = CREAR_PROCESO;
    payload_crear_proceso *payload = malloc(sizeof(payload_crear_proceso));
    payload->path = path;
    payload->pid = PID;
    instruccion->payload = payload;
    agregar_a_paquete(nuevo_paquete, instruccion, sizeof(t_paquete_entre));
    // envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
    enviar_paquete(nuevo_paquete, socketMemoria);

    eliminar_paquete(nuevo_paquete);
}

void enviar_paquete_memoria(OP_CODES_ENTRE operacion, void* payload, int socketMemoria){
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->payload = payload;
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paq,socketMemoria);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A MEMORIA");
    eliminar_paquete(paq);
    free(paquete);
}

void enviar_paquete_cpu_dispatch(OP_CODES_ENTRE operacion, void *payload, int socketDispatch)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->payload = payload;
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paq,socketDispatch);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A CPU DISPATCH");
    eliminar_paquete(paq);
    free(paquete);
}

void interrumpir(int socketInterrupt)
{
    t_paquete *paquete_fin_de_q = crear_paquete();
    t_paquete_entre *fin_q = malloc(sizeof(t_paquete_entre));
    fin_q->operacion = INTERRUMPIR_PROCESO;
    agregar_a_paquete(paquete_fin_de_q, fin_q, sizeof(t_paquete_entre));
    enviar_paquete(paquete_fin_de_q, socketInterrupt);
    eliminar_paquete(paquete_fin_de_q);
}