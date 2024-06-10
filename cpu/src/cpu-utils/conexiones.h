#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "./cicloInstruccion.h"

void enviar_pcb_kernel(t_PCB* pcb, OP_CODES_ENTRE operacion);
handshake_cpu_memoria handshake_memoria(char* ip_memoria, char* puerto_memoria);
void* solicitar_dato_memoria(int dirFisica);
int enviar_dato_memoria(int dirFisica, void* dato);
int solicitar_resize_memoria(int pid, int tam);
void solicitar_wait(char* recurso);
void solicitar_signal(char* recurso);
char* solicitar_io_stdin(int tam);