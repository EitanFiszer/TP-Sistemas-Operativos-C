//ESTRUCTURA PCB
#include <stdint.h>
#include <utils/registros.h>


typedef struct {
    int PID;
    int quantum;
    uint32_t program_counter;
    registros_t cpu_registro; 
    t_proceso_estado estado;
}t_PCB;


#include <stdint.h>
#include <utils/registros.h>


typedef struct {
    int PID;
    int quantum;
    uint32_t program_counter;
    registros_t cpu_registro; 
    t_proceso_estado estado;
}t_PCB;


#include <stdint.h>
#include <utils/registros.h>
#include "./estados.h"

typedef struct {
    int PID;
    int quantum;
    uint32_t program_counter;
    registros_t cpu_registro; 
    t_proceso_estado estado;
}t_PCB;

