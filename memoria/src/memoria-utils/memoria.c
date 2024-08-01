#include "./procesos.h"
#include <stdlib.h>
#include <commons/string.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <commons/memory.h>
#include <commons/log.h>
#include <string.h>


extern Memoria memoria; 
extern t_bitarray* marcosLibres;
extern int TAM_PAGINA;
extern t_log* logger;

int buscarDireccionFisicaEnTablaDePaginas(int pid, int pagina) {
    // printf("Buscando dirección física en tabla de páginas del proceso %d, pagina %d\n", pid, pagina);

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

    // printf("Dirección física encontrada: %d\n", marco);

    return marco;
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

void escribirMemoria(int pid, int direccionFisica, void* dato, int tamDato) {
    int numPagina = direccionFisica / TAM_PAGINA;
    int offset = direccionFisica % TAM_PAGINA;
    Proceso* proceso = procesoPorPID(pid);

    if (proceso == NULL) {
        return;
    }

    int marco = buscarDireccionFisicaEnTablaDePaginas(pid, numPagina);

    if (marco == -1) {
        return;
    }

    int offsetRestanteDelMarco = TAM_PAGINA - offset;
    int bytesEscritos = 0;
    while (bytesEscritos < tamDato) {
        int bytesAEscribir = tamDato - bytesEscritos;
        if (bytesAEscribir > offsetRestanteDelMarco) {
            bytesAEscribir = offsetRestanteDelMarco;
        }
        log_info(logger, "Escribiendo %d bytes en la dirección física %d", bytesAEscribir, marco * TAM_PAGINA + offset);
        memcpy(memoria.memoria + marco * TAM_PAGINA + offset, dato + bytesEscritos, bytesAEscribir);
        mem_hexdump(memoria.memoria + marco * TAM_PAGINA, TAM_PAGINA);
        bytesEscritos += bytesAEscribir;
        offset = 0;
        numPagina++;
        marco = buscarDireccionFisicaEnTablaDePaginas(pid, numPagina);
        if (marco == -1) {
            return;
        }
        offsetRestanteDelMarco = TAM_PAGINA;
    }
}

void* obtenerDatoMemoria(int direccion, int tamDato) {
    void* dato;
    memcpy(&dato, memoria.memoria + direccion, tamDato);
    return dato;
}
