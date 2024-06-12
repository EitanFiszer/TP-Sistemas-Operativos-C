#ifndef GLOBALES_H
#define GLOBALES_H

#include <commons/log.h>

#define MAX_RECURSOS 10
typedef struct{
    char* nombre_recurso;
    int instancias_recurso;
    pthread_mutex_t mutex_recurso;
    t_queue* cola_blocked_recurso;
}Recurso;


extern t_log *logger;
extern int puerto;
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
extern int quantum;
extern char *algoritmo_planificacion; 
extern Recurso recursos[MAX_RECURSOS];


#endif // GLOBALES_H
