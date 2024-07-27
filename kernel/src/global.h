#ifndef GLOBALES_H
#define GLOBALES_H

#include <commons/log.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <semaphore.h>

// #define MAX_RECURSOS 10


extern t_log *logger;
extern char* ip_memoria;
extern char* ip_cpu;
extern char* puerto_memoria;
extern char* puerto_escucha;
extern char* puerto_cpu_dispatch;
extern char* puerto_cpu_interrupt;
extern int resultHandshakeMemoria;
extern int resultHandshakeDispatch;
extern int resultHandshakeInterrupt;
extern int server_fd;
extern int64_t quantum;
extern char *algoritmo_planificacion; 
extern t_config* config;
extern t_dictionary* diccionario_recursos;
extern t_dictionary* rec_por_pid_dic;
extern int grado_multiprog;
extern pthread_mutex_t interrupcion_syscall;
extern bool interrumpio_syscall;

extern pthread_mutex_t sem_q_ready_priori;
extern t_queue* cola_ready_priori;
extern sem_t sem_cont_ready;
extern pthread_mutex_t sem_q_blocked;

extern bool bool_error_memoria;
extern bool bool_syscall;
extern bool bool_interrupted_by_user;
extern bool bool_interrumpi;

typedef enum {
    SYSCALL,
    FIN_QUANTUM,
    ERROR_OUT_OF_MEMORY_I,
    INTERRUPTED_BY_USER
}t_motivo_interrupcion;

// extern pthread_mutex_t logger_mutex;
// extern pthread_mutex_t printf_mutex;
// extern pthread_mutex_t consola_mutex;

// extern Recurso recursos[MAX_RECURSOS];


#endif // GLOBALES_H
