#include <commons/log.h>

typedef struct {
    char* puerto;
    t_log* logger;
} args;

void conexion_interrupt(void* argumentos);