#include <stdlib.h> 
#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/constants.h>
#include <utils/iniciar.h>
#include <string.h>
#include <pthread.h>
#include "hilos.h"

t_log* logger;
int socketMemoria;

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

    if (strcmp(interfaz, "IO_GEN") == 0) {
        funcion = hilo_generica;
    } else if (strcmp(interfaz, "IO_STDIN") == 0) {
        funcion = hilo_stdin;
    } else if (strcmp(interfaz, "IO_STDOUT") == 0) {
        funcion = hilo_stdout;
    } else if (strcmp(interfaz, "IO_DIALFS") == 0) {
        funcion = hilo_dialfs; // Define esta función si es necesario
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
    
    char* ipMemoria = config_get_string_value(config, "IP_MEMORIA");
    char* puertoMemoria = config_get_string_value(config, "PUERTO_MEMORIA");
    socketMemoria = connectAndHandshake(ipMemoria, puertoMemoria, IO, "memoria", logger);
    
    // Asegúrate de que hay un número par de argumentos
    if ((argc - 1) % 2 != 0) {
        log_error(logger, "Número incorrecto de argumentos.");
        return 1;
    }

    for (int i = 1; i < argc; i += 2) {
        log_info(logger, "Creando hilo %s", argv[i]);
        crearHilo(argv[i], argv[i+1], argv[argc-1]);
    }

    log_destroy(logger);
    return 0;
}
