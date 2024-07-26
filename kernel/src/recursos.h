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

typedef struct{
    char* nombre_recurso;
    int instancias_recurso;
}t_rec_list;






int buscar_recurso(t_list*, char*);
void recorrer_liberar_rec(t_list*);

void modificar_wait_dic_rec(int,char*);
void modificar_signal_dic_rec(int,char*);
void modificar_fin_proc_dic_rec(int); 

void guardar_dictionary_recursos(t_config* );

void atender_wait(t_PCB* , char*);

void atender_signal(t_PCB*, char*);

void remove_cola_blocked_rec(char *, t_PCB *);

    
void eliminar_diccionario_rec();


#endif /* RECURSOS_H */