#define PROCESOS_H
#include <commons/log.h>

#define MAX_PROCESOS 100

typedef struct proceso{
    int pid;
    char **instrucciones;
    int cant_instrucciones;
} Proceso;

typedef struct memoria{
    void *memoria;
    int cant_marcos;
    int *marcos;
    Proceso *procesos;
    int cant_procesos;
    int max_procesos;
} Memoria;


void inicializarMemoria(t_log* logger);
char** leer_archivo(const char* nombre_archivo, int* num_lineas);
void crearProceso(const char *nombre_archivo, int pid, t_log* logger);
void finalizarProceso(int pid, t_log* logger);
char* obtenerInstruccion(int pid, int n);

