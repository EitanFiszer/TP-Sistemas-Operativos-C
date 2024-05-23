#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "./cicloInstruccion.h"

void enviar_pcb_kernel(t_PCB* pcb, int socketKernel, OP_CODES_ENTRE operacion);