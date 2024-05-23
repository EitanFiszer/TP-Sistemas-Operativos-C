#include <commons/log.h>
#include <utils/server.h>
#include <utils/client.h>
#include <cpu-utils/conexiones.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>

int socketKernel;
registros_t registros;
t_log* logger;
int socketMemoria;

typedef struct {
    
    char* puerto;
    t_log* logger;
} args;
