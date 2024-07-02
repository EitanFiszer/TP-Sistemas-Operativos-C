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
#include <recursos.h>

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

// int PID;
bool condition_pcb_find(void*);
t_PCB* get_and_remove_pcb(int);
void desalojar();
void atender_syscall(void*);
void *manejar_quantum(void*);
// int buscar_recurso(char*);

void iniciar_proceso(char*);
void iniciar_colas();
void iniciar_semaforos();
void cargar_ready(t_PCB*, t_proceso_estado);
void cargar_ready_por_pid(int);
void stl_FIFO();
void stl_RR();
void stl_VRR();

void enviar_new_exit(int);
void lts_ex(t_PCB*,t_proceso_estado);
t_PCB *crear_PCB(int);
// void atender_wait(t_PCB*, char*);
// void atender_signal(t_PCB*, char*);

void* planificacion (void*);
#endif // PLANIFICACION_H