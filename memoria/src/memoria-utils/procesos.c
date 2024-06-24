#include <stdio.h>
#include "procesos.h"
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>
#include <utils/envios.h>

// También el RETARDO en la/s respuesta/s y concatenar PATH con el recibido de la cpu

extern int TAM_PAGINA;
extern int TAM_MEMORIA; 
extern t_log* logger;
extern char* path_instrucciones;
extern int socketKernel;

Memoria memoria;


void inicializarMemoria() {
    memoria.max_procesos = TAM_MEMORIA/TAM_PAGINA;

    memoria.memoria = malloc(TAM_MEMORIA);
    memoria.marcos = malloc(memoria.max_procesos * sizeof(int));
    memoria.procesos = malloc(memoria.max_procesos * sizeof(Proceso));
    memoria.cant_procesos = 0;

    for (int i = 0; i < memoria.max_procesos; i++) {
        memoria.marcos[i] = -1;
    }

    log_info(logger, "Se inicializó la memoria con un tamaño de %d y %d marcos", TAM_MEMORIA, 32);
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
        char linea[32];
        char** lineas = malloc(1000 * sizeof(char*)); //Puse 1000 lineas de tamanio
        int contador = 0;

        while (fgets(linea, sizeof(linea), archivo)) {
            lineas[contador] = sacar_salto_linea(linea);
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

    char* path_archivo = malloc(strlen(path_instrucciones) + strlen(nombre_archivo) + 1);
    strcpy(path_archivo, path_instrucciones);
    strcat(path_archivo, nombre_archivo);

    Proceso *proceso = &memoria.procesos[memoria.cant_procesos++];
    proceso->pid = pid;
    proceso->instrucciones = leer_archivo(path_archivo, &proceso->cant_instrucciones);

    log_info(logger, "Se creó el proceso con ID: %d", pid);

    enviar_paquete_entre(socketKernel, INSTRUCCIONES_CARGADAS, &pid, sizeof(int));
}

void finalizarProceso(int pid) {
    int indice = -1;

    for (int i = 0; i < memoria.cant_procesos; i++) {
        if (memoria.procesos[i].pid == pid) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        log_info(logger, "No se encontró el proceso con pid %d", pid);
        return;
    }

    Proceso* proceso = &memoria.procesos[indice];

    for (int i = 0; i < 32; i++) {
        if (memoria.marcos[i] == pid) {
            memoria.marcos[i] = -1;
        }
    }

    for (int i = 0; i < proceso->cant_instrucciones; i++) {
        free(proceso->instrucciones[i]);
    }
    free(proceso->instrucciones);

    for (int i = indice; i < memoria.cant_procesos - 1; i++) {
        memoria.procesos[i] = memoria.procesos[i + 1];
    }
    memoria.cant_procesos--;

    log_info(logger, "Se finalizó el proceso con ID: %d", pid);
}

// TODO: REVISAR ESTA FUNCIÓN
char* obtenerInstruccion(int pid, int n) {
    int indice = -1;

    for (int i = 0; i < memoria.cant_procesos; i++) {
        if (memoria.procesos[i].pid == pid) {
            indice = i;
            break;
        }
    }

    if (indice == -1) {
        return NULL;
    }

    Proceso* proceso = &memoria.procesos[indice];

    if (n < 0 || n >= proceso->cant_instrucciones) {
        return NULL;
    }

    return proceso->instrucciones[n];
}
