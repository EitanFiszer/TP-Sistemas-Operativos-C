#include <stdio.h>
#include "procesos.h"
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

struct proceso_t {
    int id;
};


char** leer_archivo(const char *nombre_archivo, t_log* logger, int* num_lineas) {
    FILE *archivo = fopen(nombre_archivo, "r");

    if (!archivo) {
        log_info(logger, "No se pudo abrir el archivo");
        exit(1);
    } else {
        log_info(logger, "Archivo abierto correctamente");
        char buffer[1024]; // Tamaño máximo de una línea, ajusta según necesidad
        char** lineas = NULL;
        int capacidad = 0;
        int contador = 0;

        while (fgets(buffer, sizeof(buffer), archivo)) {
            if (contador >= capacidad) {
                capacidad += 10; // Ajusta según necesidad
                lineas = realloc(lineas, capacidad * sizeof(char*));
                if (!lineas) {
                    // Error al asignar memoria
                    fclose(archivo);
                    exit(1);
                }
            }
            lineas[contador] = strdup(buffer);
            contador++;
        }

        fclose(archivo);
        *num_lineas = contador;
        return lineas;
    }
}

//Al crear un proceso, la memoria de instrucciones debe leer el archivo de pseudocódigo y devolver sus intrucciones, de a una, según el CPU lo requiera mediante el program counter.
proceso_t* crear_proceso(int id, const char* nombre_archivo, t_log* logger) {
    proceso_t* proceso = (proceso_t*)malloc(sizeof(proceso_t));
    proceso->id = id;
    log_info(logger, "Se creó un proceso con ID: %d", id);

    int num_lineas;
    char** lineas = leer_archivo(nombre_archivo, logger, &num_lineas);

    for (int i = 0; i < num_lineas; i++) {
        log_info(logger, lineas[i]);
        free(lineas[i]);
    }
    free(lineas);

    return proceso;
}
