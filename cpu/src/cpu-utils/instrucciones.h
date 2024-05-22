#include "../../kernel/src/kernel-utils/PCB.h"

int valorDelRegistro(char* reg);
void instruccionSet(t_PCB* pcb, char* reg, int valor);
void instruccionSum(t_PCB* pcb, char* regDest, char* regOrig);
void instruccionSub(t_PCB* pcb, char* regDest, char* regOrig);
void instruccionJNZ(t_PCB* pcb, char* regDato, int instruccionASaltar);
void instruccionIoGenSleep(t_PCB* pcb, char* interfaz, int tiempo);