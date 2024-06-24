#include "inicios.h"
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./conexiones.h"
#include <pthread.h>

extern t_log* logger;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern int socketCpu;
extern int socketKernel;

extern pthread_t hiloEsperaCpu;
extern pthread_t hiloEsperaKernel;
extern pthread_t hiloEsperaIO;

t_bitarray* iniciarBitarray(char* string) {
    int tamBitArray = TAM_MEMORIA / TAM_PAGINA;
    int tamString = (int)ceil((double)tamBitArray / 8);
    int tamStringBytes;

    if (tamString % 8 == 0) {
        tamStringBytes = tamString;
    } else {
        tamStringBytes = tamString + 1;
    }

    string = malloc(tamStringBytes);
    memset(string, 0, tamStringBytes);


    t_bitarray* frames = bitarray_create_with_mode(string, (tamBitArray/8), MSB_FIRST);
    log_info(logger, "Se inicializó el bitarray con un tamaño de %d", (int)bitarray_get_max_bit(frames));

    for (int i = 0; i < tamBitArray; i++) {
        bitarray_set_bit(frames, i);
    }

    return frames;
}

void iniciarHilos() {

    pthread_create(&hiloEsperaCpu, NULL, (void*)esperar_paquetes_cpu, NULL);   
    pthread_create(&hiloEsperaKernel, NULL, (void*)esperar_paquetes_kernel, NULL);
    // pthread_create(&hiloEsperaIO, NULL, (void*)esperar_paquetes_io, NULL);

    // pthread_detach(hiloEsperaCpu);
    // pthread_detach(hiloEsperaKernel);
    // pthread_detach(hiloEsperaIO);
}