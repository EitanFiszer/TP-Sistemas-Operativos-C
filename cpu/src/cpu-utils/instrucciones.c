#include "instrucciones.h"

int fetchInstruccion(t_PCB* pcb, int socketMemoria, char* instruccionRecibida) {
    // 1. Enviar PID y PC a Memoria
    int pid = pcb->PID;
    uint32_t pc = pcb->program_counter;
    
    // 1.1 Crear paquete entre CPU y Memoria
    t_paquete* paquete = crear_paquete();

    t_paquete_entre* instruccion = malloc(sizeof(t_paquete_entre));
    t_payload_fetch_instruccion* payload = malloc(sizeof(t_payload_fetch_instruccion));

    instruccion->operacion = PC_A_INSTRUCCION;
    payload->PID = pid;
    payload->program_counter = pc;
    instruccion->payload = payload;

    // 1.2 Agregar PID y PC al paquete
    agregar_a_paquete(paquete, instruccion, sizeof(t_paquete_entre));

    // 1.3 Enviar paquete
    enviar_paquete(paquete, socketMemoria);

    // 2. Recibir instruccion de Memoria
    // 2.1 Recibir paquete
    t_list* paqueteInstruccion = recibir_paquete(socketMemoria);
    if (paqueteInstruccion == NULL) {
        return -1;
    }

    // 2.2 Extraer instruccion
    t_paquete_entre* paqueteRecibido = list_get(paqueteInstruccion, 0);
    if (paqueteRecibido == NULL) {
        return -1;
    }

    instruccionRecibida = paqueteRecibido->payload;

    // 5. Retornar 0 si todo salio bien, -1 si hubo un error
    if (instruccionRecibida == NULL) {
        return -1;
    }

    return 0;
}