#include <stdlib.h> 
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/memory.h>
#include <utils/constants.h>
#include <utils/iniciar.h>
#include <string.h>
#include <pthread.h>
#include "hilos.h"
#include "bitmap.h"
#include "operacionesFS.h"
#include <dirent.h>
#include <utils.h>
#include <diccionario.h>
 
t_log* logger;
char* ip_kernel;
char* ip_memoria;
char* puerto_kernel;
char* puerto_memoria;

char* path_base2;
int block_count2;
int block_size2;

void crearHilo(char* nombre, char* path_config, char* ultimo_path) {
    t_config* config = iniciar_config(path_config);
    if (config == NULL) {
        log_error(logger, "No se pudo cargar el archivo de configuración: %s", path_config);
        return;
    }

    char* interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    if (interfaz == NULL) {
        log_error(logger, "El archivo de configuración no contiene la clave TIPO_INTERFAZ");
        config_destroy(config);
        return;
    }

    void* funcion = NULL;

    if (strcmp(interfaz, "GENERICA") == 0) {
        funcion = hilo_generica;
    } else if (strcmp(interfaz, "STDIN") == 0) {
        funcion = hilo_stdin;
    } else if (strcmp(interfaz, "STDOUT") == 0) {
        funcion = hilo_stdout;
    } else if (strcmp(interfaz, "DIALFS") == 0) {
        funcion = hilo_dialfs;
    } else {
        log_error(logger, "Tipo de interfaz desconocido: %s", interfaz);
        config_destroy(config);
        return;
    }

    if (funcion != NULL) {
        pthread_t hilo;
        args* argumentos_interrupt = malloc(sizeof(args));
        argumentos_interrupt->nombre = nombre;
        argumentos_interrupt->path_config = path_config;

        pthread_create(&hilo, NULL, funcion, (void*)argumentos_interrupt);
        if(strcmp(path_config, ultimo_path)==0){
            pthread_join(hilo,NULL);
        }else{
            pthread_detach(hilo);
        }
    }else{
        log_error(logger, "funcion no encontrada");
    }

    config_destroy(config);
}


int main(int argc, char* argv[]) {
    logger = log_create("entradasalida.log", "Entrada_Salida", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("EntradaSalida.config");
    
    //lee las ip
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    ip_kernel = config_get_string_value(config, "IP_KERNEL");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    
    //Número par de argumentos
    if ((argc - 1) % 2 != 0) {
        log_error(logger, "Número incorrecto de argumentos. %d args", argc);
        for (int i = 0; i < argc; i++) {
            log_error(logger, "Arg %d: %s", i, argv[i]);
        }

        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        log_info(logger, "Creando hilo %s", argv[i]);
        crearHilo(argv[i], argv[i+1], argv[argc-1]);
    }
    
/*
    path_base2="/FS/";
    block_size2=8;
    block_count2=16;
    inicializar_FS();
    

    leerbitmap();
    leerDiccionario();
    crear_archivo("A");
    truncate_archivo("A",16,1000,1);
    crear_archivo("B");
    truncate_archivo("B",24,1000,1);
    crear_archivo("C");
    truncate_archivo("C",24,1000,1);
    crear_archivo("D");
    truncate_archivo("D",24,1000,1);
    crear_archivo("E");
    truncate_archivo("E",16,1000,1);
    truncate_archivo("B",16,1000,1);
    truncate_archivo("C",16,1000,1);

    escribir_archivo("A",0,16,"AAAAAAAAAAAAAAAA");
    escribir_archivo("B",0,16,"BBBBBBBBBBBBBBBB");
    escribir_archivo("C",0,16,"CCCCCCCCCCCCCCCC");
    escribir_archivo("D",0,24,"DDDDDDDDDDDDDDDDDDDD");
    escribir_archivo("E",0,16,"EEEEEEEEEEEEEEEE");
    */
    log_destroy(logger);
    return 0;
}


