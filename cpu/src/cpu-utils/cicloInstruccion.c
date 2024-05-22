#include "cicloInstruccion.h"
#include <commons/string.h>
#include <utils/server.h>

int fetchInstruccion(t_PCB* pcb, int socketMemoria, char** instruccionRecibida, t_log* logger) {
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

    log_info(logger, "PID: %d - FETCH - Program Counter: %d", pid, pc);

    return 0;
}

instruccionCPU_t *dividirInstruccion(char *instruccion) {
    instruccionCPU_t* instruccionCPU = malloc(sizeof(instruccionCPU_t));
    char** instruccionSplit = string_split(instruccion, " ");
    instruccionCPU->instruccion = instruccionSplit[0];

    char** resto = instruccionSplit + 1;
    instruccionCPU->parametros = resto;

    return instruccionCPU;
}

void ejecutarInstruccion(instruccionCPU_t* instruccion, t_PCB* pcb, t_log* logger, registros_t* registros) {
    char** params = instruccion->parametros;

    char* paramsString = string_new();
    for (int i = 0; i < sizeof(params); i++) {
        string_append(&paramsString, params[i]);
        string_append(&paramsString, ", ");
    }

    log_info(logger, "PID: %d - Ejecutando: %s - %s", pcb->PID, instruccion->instruccion, paramsString);

    if(string_equals_ignore_case(instruccion->instruccion, "SET")) {
        instruccionSet(&pcb, params[0], params[1]);
    } else if(string_equals_ignore_case(instruccion->instruccion, "SUM")){
        instruccionSum(&pcb, params[0], params[1]);
    } else if (string_equals_ignore_case(instruccion->instruccion, "SUB")) {
        instruccionSub(&pcb, params[0], params[1]);
    } else if(string_equals_ignore_case(instruccion->instruccion, "JNZ")) {
        instruccionJNZ(&pcb, params[0], params[1]);
    } else if(string_equals_ignore_case(instruccion->instruccion, "IO_GEN_SLEEP")) {
        instruccionIoGenSleep(&pcb, params[0], params[1]);
    }
}