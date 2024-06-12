#include <commons/log.h>
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


void inicializarMemoria();
char** leer_archivo(const char* nombre_archivo, int* num_lineas);
void crearProceso(char *nombre_archivo, int pid);
void finalizarProceso(int pid);
char* obtenerInstruccion(int pid, int n);

