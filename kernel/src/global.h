#ifndef GLOBALES_H
#define GLOBALES_H

#include <commons/log.h>
#include <commons/collections/queue.h>
#include <pthread.h>
<<<<<<< HEAD
#include <commons/config.h>
=======
#include <commons/collections/dictionary.h>
>>>>>>> c96341f660d61b700138a1e02f9b54f64dc1de17

// #define MAX_RECURSOS 10


extern t_log *logger;
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
extern int64_t quantum;
extern char *algoritmo_planificacion; 
<<<<<<< HEAD
extern t_config* config;
=======
extern t_dictionary* diccionario_recursos;
extern int grado_multiprog;
>>>>>>> c96341f660d61b700138a1e02f9b54f64dc1de17
// extern pthread_mutex_t logger_mutex;
// extern pthread_mutex_t printf_mutex;
// extern pthread_mutex_t consola_mutex;

// extern Recurso recursos[MAX_RECURSOS];


#endif // GLOBALES_H
