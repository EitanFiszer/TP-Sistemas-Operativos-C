#include <stdio.h>
#include "procesos.h"
#include "memoria.h"
#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>
#include <string.h>
#include <utils/envios.h>
#include <math.h>

// También el RETARDO en la/s respuesta/s y concatenar PATH con el recibido de la cpu

extern int TAM_PAGINA;
extern int TAM_MEMORIA; 
extern t_log* logger;
extern char* path_instrucciones;
extern int socketKernel;
extern t_bitarray *marcosLibres;

extern Memoria memoria;
extern pthread_mutex_t mutexMemoria;

void inicializarMemoria() {
    pthread_mutex_init(&mutexMemoria, NULL);
    memoria.max_procesos = TAM_MEMORIA/TAM_PAGINA;

    memoria.memoria = malloc(TAM_MEMORIA);
    memoria.procesos = malloc(memoria.max_procesos * sizeof(Proceso));
    memoria.cant_procesos = 0;

    // llenar memoria.memoria de 0
    memset(memoria.memoria, 0, TAM_MEMORIA);

    log_info(logger, "Se inicializó la memoria con un tamaño de %d, páginas de %d bytes y %d marcos", TAM_MEMORIA, TAM_PAGINA, memoria.max_procesos);
}

char* sacar_salto_linea(char* linea) {
    char* pos;
    if ((pos = strchr(linea, '\n')) != NULL) {
        *pos = '\0';
    }
    return linea;
}

char** leer_archivo(const char *path_archivo, int* num_lineas) {
    FILE *archivo = fopen(path_archivo, "r");

    if (!archivo) {
        exit(1);
    } else {
        char linea[255];
        char** lineas = malloc(1000 * sizeof(char*)); //Puse 1000 lineas de tamanio
        int contador = 0;

        while (fgets(linea, sizeof(linea), archivo)) {
            char* lineaLeida = sacar_salto_linea(linea);
            lineas[contador] = malloc(strlen(lineaLeida) + 1);
            strcpy(lineas[contador], lineaLeida);

            // printf("Linea %d: %s\n", contador, lineas[contador]);
            contador++;
            if (contador >= 1000 || feof(archivo)) { break; }
        }

        fclose(archivo);
        *num_lineas = contador;
        return lineas;
    }
}

void crearProceso(char* nombre_archivo, int pid) {
    if (memoria.cant_procesos >= memoria.max_procesos) {
        log_info(logger, "Se alcanzó la cantidad máxima de procesos");
        return;
    }
    
    char cwd[256];
    getcwd(cwd, sizeof(cwd));

    char* path_archivo = malloc(strlen(cwd) + strlen(path_instrucciones) + strlen(nombre_archivo) + 1);
    strcpy(path_archivo, cwd);
    strcat(path_archivo, path_instrucciones);
    strcat(path_archivo, nombre_archivo);

    int existeArchivo = access(path_archivo, F_OK);
    if (existeArchivo == -1) {
        log_error(logger, "No se encontró el archivo %s", path_archivo);
        enviar_paquete_entre(socketKernel, ARCHIVO_NO_ENCONTRADO, &pid, sizeof(int));
        return;
    }

    Proceso *proceso = &memoria.procesos[memoria.cant_procesos++];
    proceso->pid = pid;
    proceso->instrucciones = leer_archivo(path_archivo, &proceso->cant_instrucciones);
    proceso->tabla_de_paginas = dictionary_create();

    // log_info(logger, "Cantidad instrucciones: %d", proceso->cant_instrucciones);

    // for(int i = 0; i < proceso->cant_instrucciones; i++) {
    //     log_info(logger, "Instruccion %d: %s", i, proceso->instrucciones[i]);
    // }

    log_info(logger, "Se creó el proceso con ID: %d", pid);

    enviar_paquete_entre(socketKernel, INSTRUCCIONES_CARGADAS, &pid, sizeof(int));
}

void liberarProceso(Proceso* proceso) {
    // dictionary_destroy_and_destroy_elements(proceso->tabla_de_paginas, (void*)free);
    // dictionary_destroy(proceso->tabla_de_paginas);
    // free(proceso);
}

void finalizarProceso(int pid) {
    Proceso* proceso = procesoPorPID(pid);

    if (proceso == NULL) {
        log_error(logger, "No se encontró el proceso con ID: %d", pid);
        return;
    }

    t_dictionary* tabla_de_paginas = proceso->tabla_de_paginas;
    t_list* marcos = dictionary_elements(tabla_de_paginas);

    // Marcar como libres los marcos ocupados por el proceso
    for (int i = 0; i < list_size(marcos); i++) {
        int marco = (int)list_get(marcos, 0);
        bitarray_set_bit(marcosLibres, marco);
    }

    // mostrarMarcosLibres();

    // dictionary_destroy_and_destroy_elements(tabla_de_paginas, (void*)destroy_elemento);
    free(tabla_de_paginas);
    free(marcos);
    liberarProceso(proceso);

    log_info(logger, "Se finalizó el proceso con ID: %d", pid);
}

char* obtenerInstruccion(int pid, int n) {
    Proceso* proceso = procesoPorPID(pid);

    if (n < 0 || proceso == NULL || n >= proceso->cant_instrucciones) {
        return NULL;
    }

    return proceso->instrucciones[n];
}

int obtenerTamanoProceso(int pid) {
    Proceso* proceso = procesoPorPID(pid);

    if (proceso == NULL) {
        return -1;
    }

    return dictionary_size(proceso->tabla_de_paginas) * TAM_PAGINA;
}

int redimensionarProceso(int pid, int nuevoTam) {
    Proceso* proceso = procesoPorPID(pid);

    if (proceso == NULL || nuevoTam < 0 || nuevoTam > TAM_MEMORIA) {
        return -1;
    }

    int cantPaginas = (int)ceil((double)nuevoTam/TAM_PAGINA);
    int cantPaginasActuales = dictionary_size(proceso->tabla_de_paginas);

    if (cantPaginasActuales == cantPaginas) {
        return 0;
    }

    int cantPaginasLibres = cantidadMarcosLibres();
    int diff = cantPaginas - cantPaginasActuales;
    
    if (diff > cantPaginasLibres) {
        return -1;
    }

    if (diff > 0) {
        for (int i = 0; i < diff; i++) {
            int marco = buscarMarcoLibre();
            if (marco == -1) {
                return -1;
            }

            dictionary_put(proceso->tabla_de_paginas, string_itoa(cantPaginasActuales + i), marco);
            bitarray_clean_bit(marcosLibres, marco);
        }
    } else {
        for (int i = 0; i < -diff; i++) {
            int marco = (int)list_remove(dictionary_elements(proceso->tabla_de_paginas), 0);
            bitarray_set_bit(marcosLibres, marco);
        }
    }

    printTabladePaginas(proceso);

    return RESIZE_SUCCESS;
}

Proceso* procesoPorPID(int pid) {
    for (int i = 0; i < memoria.cant_procesos; i++) {
        if (memoria.procesos[i].pid == pid) {
            return &memoria.procesos[i];
        }
    }

    return NULL;
}

void mostrarMarcosLibres() {
    for (int i = 0; i < bitarray_get_max_bit(marcosLibres); i++) {
        printf("%d", bitarray_test_bit(marcosLibres, i));
    }
    printf("\n");
}

void printTabladePaginas(Proceso* proceso){
    printf("Tabla de páginas de proceso %d", proceso->pid);
    t_list* keys = dictionary_keys(proceso->tabla_de_paginas);
    for(int i = 0; i < list_size(keys); i++){
        char* key = list_get(keys, i);
        int value = dictionary_get(proceso->tabla_de_paginas, key);
        printf("Pagina: %s, Marco: %d\n", key, value);
    }
    list_destroy(keys);
}