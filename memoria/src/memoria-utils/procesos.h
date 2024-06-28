#include <commons/log.h>
#include <commons/collections/dictionary.h>

typedef struct {
    int pid;
    char **instrucciones;
    int cant_instrucciones;
    t_dictionary* tabla_de_paginas;
} Proceso;

typedef struct {
    int *marcos;
    void *memoria;
    int cant_marcos;
    int max_procesos;
    Proceso *procesos;
    int cant_procesos;
} Memoria;


void inicializarMemoria();
char** leer_archivo(const char* nombre_archivo, int* num_lineas);
void crearProceso(char *nombre_archivo, int pid);
void finalizarProceso(int pid);
char* obtenerInstruccion(int pid, int n);
Proceso* procesoPorPID(int pid);