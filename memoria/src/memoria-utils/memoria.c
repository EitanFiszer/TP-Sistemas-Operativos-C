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
    printf("Buscando dirección física en tabla de páginas del proceso %d, pagina %d\n", pid, pagina);

    Proceso* proceso = procesoPorPID(pid);
    if (proceso == NULL) {
        return -1;
    }

    // TODO: POR QUE MIERDA FALLA ACÁ

    char* key = string_itoa(pagina);
    bool encontrarKey(void* keyToFind) {
      return strcmp(key, keyToFind) == 0;
    };

    t_list* dict_keys = dictionary_keys(proceso->tabla_de_paginas);

    char* dict_key = list_find(dict_keys, (void*)encontrarKey);

    if (!dict_key) {
        return -1;
    }

    int marco = dictionary_get(proceso->tabla_de_paginas, key);
    free(key);

    return marco;
}

int obtenerDatoMemoria(int direccion) {
    int dato;
    memcpy(&dato, memoria.memoria + direccion, sizeof(int));
    return dato;
}

int cantidadMarcosLibres() {
    int cant = 0;
    for (int i = 0; i < bitarray_get_max_bit(marcosLibres); i++) {
        if (bitarray_test_bit(marcosLibres, i)) {
            cant++;
        }
    }
    return cant;
}

int buscarMarcoLibre() {
    for (int i = 0; i < bitarray_get_max_bit(marcosLibres); i++) {
        if (bitarray_test_bit(marcosLibres, i)) {
            return i;
        }
    }
    return -1;
}

void escribirMemoria(int direccionFisica, void* dato, int tamDato) {
    memcpy(memoria.memoria + direccionFisica, dato, tamDato);
}