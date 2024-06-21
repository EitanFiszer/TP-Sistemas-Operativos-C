#ifndef PCB_H
#define PCB_H

//ESTRUCTURA PCB
#include <stdint.h>
#include <utils/registros.h>

typedef enum {
    NEW,
    READY, 
    BLOCKED,
    EXEC,
    EXIT
}t_proceso_estado;


typedef struct {
    int PID;
    int quantum;
    uint32_t program_counter;
    registros_t cpu_registro; 
    t_proceso_estado estado;
}t_PCB;


#endif /* PCB_H */