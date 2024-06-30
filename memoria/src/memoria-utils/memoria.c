#include "./procesos.h"
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <string.h>

extern Memoria memoria; 
extern t_bitarray* marcosLibres;
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

int cantidadMarcosLibres() {
    int cant = 0;
    for (int i = 0; i < memoria.cant_marcos; i++) {
        if (bitarray_test_bit(marcosLibres, i)) {
            cant++;
        }
    }
    return cant;
}

int buscarMarcoLibre() {
    for (int i = 0; i < memoria.cant_marcos; i++) {
        if (bitarray_test_bit(marcosLibres, i)) {
            return i;
        }
    }
    return -1;
}