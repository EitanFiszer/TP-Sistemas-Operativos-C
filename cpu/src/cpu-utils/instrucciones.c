#include "./instrucciones.h"
#include <commons/string.h>

int valorDelRegistro(char* reg, registros_t* registros) {
    if(string_equals_ignore_case(reg, "AX")){
        return registros->AX;
    } else if (string_equals_ignore_case(reg, "BX")) {
        return registros->BX;
    } else if (string_equals_ignore_case(reg, "CX")) {
        return registros->CX;
    } else if (string_equals_ignore_case(reg, "DX")) {
        return registros->DX;
    } else if (string_equals_ignore_case(reg, "EAX")) {
        return registros->EAX;
    } else if (string_equals_ignore_case(reg, "EBX")) {
        return registros->EBX;
    } else if (string_equals_ignore_case(reg, "ECX")) {
        return registros->ECX;
    } else if (string_equals_ignore_case(reg, "EDX")) {
        return registros->EDX;
    } else if (string_equals_ignore_case(reg, "SI")) {
        return registros->SI;
    } else if (string_equals_ignore_case(reg, "DI")) {
        return registros->DI;
    }
    return 0;
}

void instruccionSet(t_PCB* pcb, char* reg, int valor, registros_t* registros) {
    if(string_equals_ignore_case(reg, "AX")){
        registros->AX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "BX")) {
        registros->BX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "CX")) {
        registros->CX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "DX")) {
        registros->DX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "EAX")) {
        registros->EAX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "EBX")) {
        registros->EBX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "ECX")) {
        registros->ECX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "EDX")) {
        registros->EDX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "SI")) {
        registros->SI = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "DI")) {
        registros->DI = (uint32_t)valor;
    }

    pcb->program_counter = pcb->program_counter + 1;
}

void instruccionSum(t_PCB* pcb, char* regDest, char* regOrig, registros_t* registros) {
    int valor1 = 0;
    int valor2 = 0;

    valor1 = valorDelRegistro(regDest, registros);
    valor2 = valorDelRegistro(regOrig, registros);

    instruccionSet(pcb, regDest, valor1 + valor2, registros);
    pcb->program_counter = pcb->program_counter + 1;
}

void instruccionSub(t_PCB* pcb, char* regDest, char* regOrig, registros_t* registros) {
    int valor1 = 0;
    int valor2 = 0;

    valor1 = valorDelRegistro(regDest, registros);
    valor2 = valorDelRegistro(regOrig, registros);

    instruccionSet(pcb, regDest, valor1 - valor2, registros);
    pcb->program_counter = pcb->program_counter + 1;
}

void instruccionJNZ(t_PCB* pcb, char* reg, int instruccionASaltar, registros_t* registros) {
    int valor = valorDelRegistro(reg, registros);

    if(valor != 0) {
        pcb->program_counter = instruccionASaltar;
    } else {
        pcb->program_counter = pcb->program_counter + 1;
    }
}

void instruccionIoGenSleep(t_PCB* pcb, char* interfaz, int tiempo, registros_t* registros) {
    // TODO: CONEXION A DISPOSITIVO DE I/O y esperar respuesta

    pcb->program_counter = pcb->program_counter + 1;
}