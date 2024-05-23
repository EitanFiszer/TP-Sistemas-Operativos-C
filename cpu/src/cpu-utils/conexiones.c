#include "conexiones.h"

void enviar_pcb_kernel(t_PCB *pcb, int socketKernel, OP_CODES_ENTRE operacion) {
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete=malloc(sizeof(t_paquete_entre));  

    paquete->operacion = operacion;
    paquete->payload = pcb;
    
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paq, socketKernel);
}