#ifndef PLANIFICACION_H
#define PLANIFICACION_H

#include "global.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <utils/constants.h>
#include <pthread.h>
#include "espera.h"
#include <semaphore.h>
#include <commons/temporal.h>
#include "recursos.h"

// //colas
// t_list* lista_new;
// t_queue* cola_ready;
// t_queue* cola_ready_priori;

// t_queue* cola_blocked;
// t_queue* cola_exit;
// t_queue* cola_exec;

// //semaforos de colas

// pthread_mutex_t sem_q_new;
// pthread_mutex_t sem_q_ready;
// pthread_mutex_t sem_q_ready_priori;
// pthread_mutex_t  sem_q_blocked;
// pthread_mutex_t  sem_q_exit;
// pthread_mutex_t  sem_q_exec;
// pthread_mutex_t  sem_CPU_libre;
// sem_t sem_cont_ready;

// int PID=0;

// t_temporal* tempo_quantum;

// //hilo para manejar el quantum

// pthread_t hilo_quantum;

typedef enum
{
    IOB,
    REC,
} tipo_block;

typedef struct
{
    t_PCB *pcb;
    tipo_block tipo;
    char *key;
} str_blocked;

// int PID;
bool condition_pcb_find(void *);
t_PCB *get_and_remove_pcb(int);
void desalojar(t_PCB*);
// void atender_syscall(void*);
void *manejar_quantum(void *);
// int buscar_recurso(char*);
// void hubo_syscall(t_PCB *);

void iniciar_quantum();
void modificar_quantum(t_PCB *);
void reiniciar_quantum(t_PCB *);
// void cancelar_quantum();
void iniciar_proceso(char *);
void iniciar_colas();
void iniciar_semaforos();
void cargar_ready(t_PCB *, t_proceso_estado);
void cargar_ready_priori(t_PCB *, t_proceso_estado);
void cargar_ready_por_pid(int);
void stl_FIFO();
void stl_RR();
void stl_VRR();

void enviar_new_exit(int);
void lts_ex(t_PCB *, t_proceso_estado,char*);
t_PCB *crear_PCB(int);

void add_queue_blocked(t_PCB *, tipo_block, char *);
void delete_queue_blocked(t_PCB *);

void finalizar_proceso(int);
void detener_planificacion();
void iniciar_planificacion();
void modificar_multiprogramacion(int);
void listar_procesos_por_estado();

void *planificacion(void *);

bool buscar_pcb(int);
void sacar_bloqueo(str_blocked *);

void eliminar_semaforos();
void eliminar_colas();

#endif // PLANIFICACION_H