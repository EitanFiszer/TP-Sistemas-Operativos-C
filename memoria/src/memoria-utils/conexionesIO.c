#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <stdlib.h>
#include "conexionesIO.h"

extern t_log* logger;
extern t_dictionary *interfaces_dict;

void desconectar_IO(char *nombre_io_hilo) {
    int *socketIO = dictionary_remove(interfaces_dict, nombre_io_hilo);

    if (socketIO == NULL) {
        log_error(logger, "No se pudo desconectar el I/O %s", nombre_io_hilo);
        return;
    }
    free(socketIO);
}

void agregar_interfaz(char *nombre, int socket) {
    int nueva_interfaz = socket;
    dictionary_put(interfaces_dict, nombre, &nueva_interfaz);
}