#include <semaphore.h>
#include "hilos.h"
#include <commons/config.h>
#include <unistd.h>



int tiempo_gen;
sem_t semGen;
sem_t semGEnLog;

void hilo_generica(char* path_config){
    t_config* config = config_create(path_config);
    int tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    while(1){
        sem_wait(&semGen);
        sleep(tiempo_unidad_trabajo / 1000 * tiempo_gen);
        sem_post(&semGenLog);
    }
}