#include <stdio.h>
#include "procesos.h"
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>


Memoria memoria; 

void inicializarMemoria(t_log* logger) {
    memoria.max_procesos = 10;

    memoria.memoria = malloc(65536);
    memoria.marcos = malloc(16 * sizeof(int));
    memoria.procesos = malloc(memoria.max_procesos * sizeof(Proceso));
    memoria.cant_procesos = 0;

    for (int i = 0; i < 16; i++) {
        memoria.marcos[i] = -1;
    }

    log_info(logger, "Se inicializó la memoria");
}

char** leer_archivo(const char *nombre_archivo, int* num_lineas) {
    FILE *archivo = fopen(nombre_archivo, "r");

    if (!archivo) {
        exit(1);
    } else {
        char linea[32];
        char** lineas = malloc(1000 * sizeof(char*)); //Puse 1000 lineas de tamanio
        int contador = 0;

        while (fgets(linea, sizeof(linea), archivo)) {
            lineas[contador] = strdup(linea);
            contador++;
            if (contador >= 1000 || feof(archivo)) { break; }
        }

        fclose(archivo);
        *num_lineas = contador;
        return lineas;
    }
}

void crearProceso(const char* nombre_archivo, int pid, t_log* logger) {
    if (memoria.cant_procesos >= memoria.max_procesos) {
        log_info(logger, "Se alcanzó la cantidad máxima de procesos");
        return;
    }

    Proceso *proceso = &memoria.procesos[memoria.cant_procesos++];
    proceso->pid = pid;
    proceso->instrucciones = leer_archivo(nombre_archivo, &proceso->cant_instrucciones);

    log_info(logger, "Se creó el proceso con ID: %d", pid);
}

void finalizarProceso(int pid, t_log* logger) {
    int proceso_index = -1;

    for (int i = 0; i < memoria.cant_procesos; i++) {
        if (memoria.procesos[i].pid == pid) {
            proceso_index = i;
            break;
        }
    }

    if (proceso_index == -1) {
        log_info(logger, "No se encontró el proceso con pid %d", pid);
        return;
    }

    Proceso* proceso = &memoria.procesos[proceso_index];

    for (int i = 0; i < 16; i++) {
        if (memoria.marcos[i] == pid) {
            memoria.marcos[i] = -1;
        }
    }

    for (int i = 0; i < proceso->cant_instrucciones; i++) {
        free(proceso->instrucciones[i]);
    }
    free(proceso->instrucciones);

    for (int i = proceso_index; i < memoria.cant_procesos - 1; i++) {
        memoria.procesos[i] = memoria.procesos[i + 1];
    }
    memoria.cant_procesos--;

    log_info(logger, "Se finalizó el proceso con ID: %d", pid);
}

char* obtenerInstruccion(int pid, int n) {
    int proceso_index = -1;

    for (int i = 0; i < memoria.cant_procesos; i++) {
        if (memoria.procesos[i].pid == pid) {
            proceso_index = i;
            break;
        }
    }

    if (proceso_index == -1) {
        return NULL;
    }

    Proceso* proceso = &memoria.procesos[proceso_index];

    if (n < 0 || n >= proceso->cant_instrucciones) {
        return NULL;
    }

    return proceso->instrucciones[n];
}
