#include <utils/PCB.h>

int valorDelRegistro(char* reg, registros_t* registros);
void instruccionSet(char* reg, int valor, registros_t* registros);
void instruccionSum(t_PCB* pcb, char* regDest, char* regOrig, registros_t* registros);
void instruccionSub(t_PCB* pcb, char* regDest, char* regOrig, registros_t* registros);
void instruccionJNZ(t_PCB* pcb, char* regDato, int instruccionASaltar, registros_t registros);
void instruccionIoGenSleep(t_PCB* pcb, char* interfaz, int tiempo);
void instruccionMovIn(char* regDato, char* regDest, registros_t* registros, t_PCB* pcb);
void instruccionMovOut(char* regDire, char* regDato, registros_t* registros, t_PCB* pcb);
void instruccionResize(int tam, t_PCB* pcb);
void instruccionCopyString(int tam, registros_t registros, t_PCB* pcb);
void instruccionWait(char* recurso, t_PCB* pcb);
void instruccionSignal(char* recurso, t_PCB* pcb);
void instruccionIoSTDInRead(char* interfaz, char* regDire, char* regTam, registros_t* registros, t_PCB* pcb);
void instruccionIoSTDOutWrite(char* interfaz, char* regDire, char* regTam, registros_t* registros,  t_PCB* pcb);
void instruccionIoFSCreate(char* interfaz, char* nombreArchivo, t_PCB* pcb);
void instruccionIoFSDelete(char* interfaz, char* nombreArchivo, t_PCB* pcb);
void instruccionIoFSTruncate(char* interfaz, char* nombreArchivo, char* regTam, registros_t* registros, t_PCB* pcb);
void instruccionIoFSWrite(char* interfaz, char* nombreArchivo, char* regDire, char* regTam, char* regPuntero, registros_t* registros, t_PCB* pcb);
void instruccionIoFSRead(char* interfaz, char* nombreArchivo, char* regDire, char* regTam, char* regPuntero, registros_t* registros, t_PCB* pcb);
void instruccionExit(t_PCB* pcb);