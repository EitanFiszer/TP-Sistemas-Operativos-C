#ifndef HILOS_H
#define HILOS_H

typedef struct {
    char* nombre;
    char* path_config;
} args;


void hilo_generica(void* args);

void hilo_stdin(void* args);

void hilo_stdout(void* args);

void hilo_dialfs(void* args);


#endif // HILOS_H