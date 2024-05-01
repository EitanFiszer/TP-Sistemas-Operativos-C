//ESTRUCTURA PCB
#include <stdint.h>

typedef struct {
    uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} registros_t;
//lo agrego temporal dsp lo modifico....

typedef struct{
    int PID;
    uint32_t program_counter;
    int quantum;
    registros_t cpu_registro; 
    estado_proceso estado;
    motivo_desalojo motivo;
}PCB;

