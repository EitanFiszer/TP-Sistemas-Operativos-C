#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "./cicloInstruccion.h"

void enviar_pcb_kernel(t_PCB* pcb, int socketKernel, OP_CODES_ENTRE operacion);
handshake_cpu_memoria handshake_memoria(char* ip_memoria, char* puerto_memoria);