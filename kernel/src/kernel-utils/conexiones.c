#include <conexiones.h>

//funciones de conexion
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
void esperar_paquetes_cpu_dispatch(int socketDispatch)
{
    while (1)
    {
        t_list *paquete = recibir_paquete(socketDispatch);
        t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
        switch (paquete_dispatch->operacion)
        {
        case INTERRUMPIO_PROCESO:
            // desalojar(paquete_dispatch->payload);
            break;
        case SYSCALL:
            break;
        case TERMINO_EJECUCION:
            ///ENVIAR A
            break;
        default:
            log_error(logger, "no se recibio paquete de la CPU, error");
            break;
        }
    }
}
void esperar_paquetes_memoria(int socketMemoria)
{
    while (1)
    {
        t_list *paquete = recibir_paquete(socketMemoria);
        t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
        switch (paquete_dispatch->operacion)
        {
        case INSTRUCCIONES_CARGADAS:
            // permite continuar con la planificacion a largo plazo
            break;
        default:
            log_error(logger, "no se recibio paquete de la memoria, error");
            break;
        }
    }
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
    t_paquete *paquete_fin_de_q = crear_paquete;
    t_paquete_entre *fin_q = malloc(sizeof(t_paquete_entre));
    fin_q->operacion = INTERRUMPIR_PROCESO;
    agregar_a_paquete(paquete_fin_de_q, fin_q, sizeof(t_paquete_entre));
    enviar_paquete(paquete_fin_de_q, socketInterrupt);
    eliminar_paquete(paquete_fin_de_q);
}

