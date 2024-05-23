#include <utils/PCB.h>

int valorDelRegistro(char* reg, registros_t* registros);
void instruccionSet(t_PCB* pcb, char* reg, int valor, registros_t* registros);
void instruccionSum(t_PCB* pcb, char* regDest, char* regOrig, registros_t registros);
void instruccionSub(t_PCB* pcb, char* regDest, char* regOrig, registros_t registros);
void instruccionJNZ(t_PCB* pcb, char* regDato, int instruccionASaltar, registros_t registros);
void instruccionIoGenSleep(t_PCB* pcb, char* interfaz, int tiempo, int socketKernel);