#include <utils/client.h>
#include <utils/server.h>
#include <utils/registros.h>
#include <commons/collections/list.h>
#include "./instrucciones.h"

typedef struct {
    char* instruccion;
    char** parametros;
} instruccionCPU_t;

int fetchInstruccion(t_PCB* pcb, int socketMemoria, char** instruccionRecibida, t_log* logger);
instruccionCPU_t* dividirInstruccion(char* instruccion);
void ejecutarInstruccion(instruccionCPU_t* instruccionCPU, t_PCB* pcb, int socketMemoria, t_log* logger, registros_t* registros);
