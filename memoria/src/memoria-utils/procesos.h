#define PROCESOS_H
#include <commons/log.h>

#define MAX_PROCESOS 100

typedef struct proceso {
    int id;
} proceso_t;

typedef struct {
    proceso_t* procesos[MAX_PROCESOS];
} memoria_t;

proceso_t* crear_proceso(int id, const char* nombre_archivo, t_log* logger);
char** leer_archivo(const char* nombre_archivo, t_log* logger, int* num_lineas);
