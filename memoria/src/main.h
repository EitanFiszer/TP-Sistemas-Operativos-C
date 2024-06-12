#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <memoria-utils/procesos.h>
#include <memoria-utils/inicios.h>
#include <semaphore.h>
#include "./memoria-utils/conexiones.h"

t_log* logger;
t_config* config;
t_bitarray *bitarray;

void liberarMemoria();