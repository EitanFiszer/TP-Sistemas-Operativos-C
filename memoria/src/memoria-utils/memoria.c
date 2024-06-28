#include "./procesos.h"
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>

extern Memoria memoria; 
extern int TAM_PAGINA;

int buscarDireccionFisicaEnTablaDePaginas(int pid, int pagina) {
    Proceso* proceso = procesoPorPID(pid);
    if (proceso == NULL) {
        return -1;
    }

    char* key = string_itoa(pagina);
    int* marco = dictionary_get(proceso->tabla_de_paginas, key);
    free(key);

    if (marco == NULL) {
        return -1;
    }

    return *marco;
}

int obtenerDatoMemoria(int direccion) {
    int dato;
    memcpy(&dato, memoria.memoria + direccion, sizeof(int));
    return dato;
}