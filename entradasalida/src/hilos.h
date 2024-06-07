#ifndef HILOS_H
#define HILOS_H

#include <semaphore.h>

extern int tiempo_gen;
extern sem_t semGen;
extern sem_t semGenLog;

void hilo_generica(char* path_config);

#endif // HILOS_H