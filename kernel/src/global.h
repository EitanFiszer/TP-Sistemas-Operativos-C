#ifndef GLOBALES_H
#define GLOBALES_H

#include <commons/log.h>

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



#endif // GLOBALES_H
