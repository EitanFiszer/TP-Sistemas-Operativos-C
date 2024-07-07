#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <utils/constants.h>
#include "./cicloInstruccion.h"
#include <utils/serializacion.h>
#include <utils/envios.h>

void enviar_pcb_kernel(t_PCB* pcb, OP_CODES_ENTRE operacion);
handshake_cpu_memoria handshake_memoria(char* ip_memoria, char* puerto_memoria);
void* solicitar_dato_memoria(int dirFisica);
int enviar_dato_memoria(int dirFisica, void* dato, int tamDato);
int solicitar_resize_memoria(int pid, int tam);
void solicitar_wait(char* recurso, t_PCB* pcb);
void solicitar_signal(char* recurso, t_PCB* pcb);
void solicitar_io_stdin(int tam, t_PCB* pcb, char* interfaz, char* regTam, int dirFisica);
void solicitar_io_stdout(char* interfaz, int direccionFisica, int tam, t_PCB* pcb);
void solicitar_fs_createORdelete(char* interfaz, char* nombreArchivo, OP_CODES_ENTRE oper, t_PCB* pcb);
void solicitar_fs_truncate(char* interfaz, char* nombreArchivo, char* regTam, t_PCB* pcb);
void solicitar_fs_writeORread(char* interfaz, char* nombreArchivo, char* regTam, char* regDire, char* regPuntero, OP_CODES_ENTRE oper, t_PCB* pcb);


