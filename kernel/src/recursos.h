#ifndef RECURSOS_H
#define RECURSOS_H

#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <commons/config.h>
#include <utils/constants.h>
#include "planificacion.h"

typedef struct{
    char* nombre_recurso;
    int instancias_recurso;
    pthread_mutex_t* mutex_recurso;
    t_queue* cola_blocked_recurso;
}t_recurso;


void guardar_dictionary_recursos(t_config* );

void atender_wait(t_PCB* , char*);

void atender_signal(t_PCB*, char*);



#endif /* RECURSOS_H */