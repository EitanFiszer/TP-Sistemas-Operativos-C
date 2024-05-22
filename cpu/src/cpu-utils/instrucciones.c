#include "./instrucciones.h"
#include <commons/string.h>
#include "main.h"

int valorDelRegistro(char* reg) {
    if(string_equals_ignore_case(reg, "AX")){
        return registros.AX;
    } else if (string_equals_ignore_case(reg, "BX")) {
        return registros.BX;
    } else if (string_equals_ignore_case(reg, "CX")) {
        return registros.CX;
    } else if (string_equals_ignore_case(reg, "DX")) {
        return registros.DX;
    } else if (string_equals_ignore_case(reg, "EAX")) {
        return registros.EAX;
    } else if (string_equals_ignore_case(reg, "EBX")) {
        return registros.EBX;
    } else if (string_equals_ignore_case(reg, "ECX")) {
        return registros.ECX;
    } else if (string_equals_ignore_case(reg, "EDX")) {
        return registros.EDX;
    } else if (string_equals_ignore_case(reg, "SI")) {
        return registros.SI;
    } else if (string_equals_ignore_case(reg, "DI")) {
        return registros.DI;
    }
    return 0;
}

// Asigna al registro el valor pasado como parámetro
void instruccionSet(t_PCB* pcb, char* reg, int valor) {
    if(string_equals_ignore_case(reg, "AX")){
        registros.AX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "BX")) {
        registros.BX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "CX")) {
        registros.CX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "DX")) {
        registros.DX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "EAX")) {
        registros.EAX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "EBX")) {
        registros.EBX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "ECX")) {
        registros.ECX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "EDX")) {
        registros.EDX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "SI")) {
        registros.SI = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "DI")) {
        registros.DI = (uint32_t)valor;
    }

    pcb->program_counter = pcb->program_counter + 1;
}

// Suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino
void instruccionSum(t_PCB* pcb, char* regDest, char* regOrig) {
    int valor1 = 0;
    int valor2 = 0;

    valor1 = valorDelRegistro(regDest);
    valor2 = valorDelRegistro(regOrig);

    instruccionSet(pcb, regDest, valor1 + valor2);
    pcb->program_counter = pcb->program_counter + 1;
}

// Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino
void instruccionSub(t_PCB* pcb, char* regDest, char* regOrig) {
    int valor1 = 0;
    int valor2 = 0;

    valor1 = valorDelRegistro(regDest);
    valor2 = valorDelRegistro(regOrig);

    instruccionSet(pcb, regDest, valor1 - valor2);
    pcb->program_counter = pcb->program_counter + 1;
}

// Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro
void instruccionJNZ(t_PCB* pcb, char* reg, int instruccionASaltar) {
    int valor = valorDelRegistro(reg);

    if(valor != 0) {
        pcb->program_counter = instruccionASaltar;
    } else {
        pcb->program_counter = pcb->program_counter + 1;
    }
}

// Esta instrucción solicita al Kernel que se envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo
void instruccionIoGenSleep(t_PCB* pcb, char* interfaz, int tiempo ) {
    socketKernel;

    pcb->program_counter = pcb->program_counter + 1;
}