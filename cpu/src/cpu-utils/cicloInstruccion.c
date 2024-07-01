#include "cicloInstruccion.h"
#include <commons/string.h>
#include <utils/server.h>
#include <utils/envios.h>
#include <utils/serializacion.h>

extern int socketKernel;
extern registros_t registros;

int fetchInstruccion(t_PCB* pcb, int socketMemoria, char** instruccionRecibida, t_log* logger) {
    // 1. Enviar PID y PC a Memoria
    int pid = pcb->PID;
    uint32_t pc = pcb->program_counter;

    // 1.1 Crear paquete entre CPU y Memoria
    t_payload_pc_a_instruccion* payload = malloc(sizeof(t_payload_pc_a_instruccion));
    payload->PID = pid;
    payload->program_counter = pc;

    enviar_paquete_entre(socketMemoria, PC_A_INSTRUCCION, payload, sizeof(t_payload_pc_a_instruccion));

    // 2. Recibir instruccion de Memoria
    // 2.1 Recibir paquete
    t_paquete_entre* paqueteRecibido = recibir_paquete_entre(socketMemoria);
    if (paqueteRecibido == NULL) {
        free(paqueteRecibido);
        return -1;
    }

    if (paqueteRecibido->operacion == FIN_DE_INSTRUCCIONES) {
        free(paqueteRecibido);
        return -1;
    } else if ( paqueteRecibido->operacion != GET_INSTRUCCION) {
        free(paqueteRecibido);
        return -1;
    }

    // 2.2 Extraer instruccion
    t_payload_get_instruccion* payloadRecibido = deserializar_get_instruccion(paqueteRecibido->payload);
    *instruccionRecibida = payloadRecibido->instruccion;

    // 5. Retornar 0 si todo salio bien, -1 si hubo un error
    if (instruccionRecibida == NULL) {
        return -1;
    }

    log_info(logger, "PID: %d, Program Counter: %d - FETCH - Instruccion: %s", pid, pc, *instruccionRecibida);

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

void ejecutarInstruccion(instruccionCPU_t* instruccion, t_PCB* pcb, t_log* logger, int socketKernel) {
    char** params = instruccion->parametros;
    char* inst = instruccion->instruccion;

    char* paramsString = string_new();

    int idx = 0;
    while(params[idx]) {
        // string_append(&paramsString, params[i]);
        // string_append(&paramsString, ", ");

        string_append_with_format(&paramsString, "%s, ", params[idx++]);
    }

    log_info(logger, "PID: %d - Ejecutando: %s - %s", pcb->PID, inst, paramsString);

    if(string_equals_ignore_case(inst, "SET")) {
        instruccionSet(params[0], atoi(params[1]), &registros);
        pcb->program_counter++;
    } else if(string_equals_ignore_case(inst, "SUM")){
        instruccionSum(pcb, params[0], params[1], registros);
    } else if(string_equals_ignore_case(inst, "SUB")) {
        instruccionSub(pcb, params[0], params[1], registros);
    } else if(string_equals_ignore_case(inst, "JNZ")) {
        instruccionJNZ(pcb, params[0], (intptr_t)params[1], registros);
    } else if(string_equals_ignore_case(inst, "IO_GEN_SLEEP")) {
        instruccionIoGenSleep(pcb, params[0], (intptr_t)params[1]);
    } else if(string_equals_ignore_case(inst, "MOV_IN")) { // MOV_IN EDX ECX
        instruccionMovIn(params[0], params[1], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "MOV_OUT")) { // MOV_OUT
        instruccionMovOut(params[0], params[1], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "RESIZE")) { // RESIZE 128
        instruccionResize(atoi(params[0]), pcb);
    } else if(string_equals_ignore_case(inst, "COPY_STRING")) { // COPY_STRING 8
        instruccionCopyString((intptr_t)params[0], registros, pcb);
    } else if(string_equals_ignore_case(inst, "WAIT")) { // WAIT RECURSO_1
        instruccionWait(params[0], pcb);
    } else if(string_equals_ignore_case(inst, "SIGNAL")) { // SIGNAL RECURSO_1
        instruccionSignal(params[0], pcb);
    } else if(string_equals_ignore_case(inst, "IO_STDIN_READ")) { // IO_STDIN_READ Int2 EAX AX
        instruccionIoSTDInRead(params[0], params[1], params[2], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "IO_STDOUT_WRITE")) { // IO_STDOUT_WRITE Int3 BX EAX 
        instruccionIoSTDOutWrite(params[0], params[1], params[2], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "IO_FS_CREATE")) { // IO_FS_CREATE Int4 notas.txt  
        instruccionIoFSCreate(params[0], params[1], pcb);
    } else if(string_equals_ignore_case(inst, "IO_FS_DELETE")) { // IO_FS_DELETE Int4 notas.txt
        instruccionIoFSDelete(params[0], params[1], pcb);
    } else if(string_equals_ignore_case(inst, "IO_FS_TRUNCATE")) { // IO_FS_TRUNCATE Int4 notas.txt ECX  
        instruccionIoFSTruncate(params[0], params[1], params[2], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "IO_FS_WRITE")) { // IO_FS_WRITE Int4 notas.txt AX ECX EDX
        instruccionIoFSWrite(params[0], params[1], params[2], params[3], params[4], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "IO_FS_READ")) { // IO_FS_READ Int4 notas.txt BX ECX EDX
        instruccionIoFSRead(params[0], params[1], params[2], params[3], params[4], &registros, pcb);
    } else if(string_equals_ignore_case(inst, "EXIT")) { // EXIT
        instruccionExit(pcb);
    } 
    pcb->cpu_registro = registros;
}

void ejecutarCicloCompleto(t_PCB* pcb) {
}