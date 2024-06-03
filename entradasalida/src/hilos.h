#include <semaphore.h>

int tiempo_gen;
sem_t semGen;
sem_t semGenLog;


void hilo_generica(char* path_config);