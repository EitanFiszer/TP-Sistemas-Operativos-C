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
extern int server_fd;

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

void esperar_clientes_io(void *args) {
    while (1) {
        pthread_t hilo_atender_cliente;
        int *fd_conexion_ptr = malloc(sizeof(int));

        Handshake res = esperar_cliente(server_fd, logger);
        int modulo = res.modulo;
        *fd_conexion_ptr = res.socket;
        switch (modulo) {
            case IO:
                log_info(logger, "Se conecto un I/O con el socket %d", *fd_conexion_ptr);
                pthread_create(&hilo_atender_cliente, NULL, (void *)atender_cliente_io, fd_conexion_ptr);
                pthread_detach(hilo_atender_cliente);
                break;
            default:
                break;
        }
    }
}