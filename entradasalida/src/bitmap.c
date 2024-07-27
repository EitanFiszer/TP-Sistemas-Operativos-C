#include "bitmap.h"
#include <commons/bitarray.h>
#include <commons/log.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include "utils.h"

extern t_log* logger;

void crear_bitmap(int block_count) {
    int tamString = ceil((double)block_count / 8);
    int tamStringBytes;

    if (block_count < 8)
        tamStringBytes = 1;
    else {
        tamStringBytes = tamString;
    }

    char* string = malloc(block_count);
    memset(string, 0, tamStringBytes);

    t_bitarray* bitarray = bitarray_create_with_mode(string, tamStringBytes, MSB_FIRST);

    log_info(logger, "Se inicializó el bitarray con un tamaño de %lu", bitarray_get_max_bit(bitarray));

    // Crear la ruta completa para el archivo bitmap.dat
    FILE* archivo = crear_archivo_fs("bitmap.dat");

    if (!(fwrite(bitarray, sizeof(t_bitarray), 1, archivo))) {
        perror("Error al escribir en el archivo de bitmap");
        bitarray_destroy(bitarray);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    bitarray_destroy(bitarray);
    fclose(archivo);
}

int getBit(int block_count) {
    // Crear la ruta completa para el archivo bitmap.dat
    char* path_archivo = crear_ruta("bitmap.dat");
    t_bitarray* bitarray;

    FILE* archivo = fopen(path_archivo, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        return -1;
    }

    // Leer el bitmap desde el archivo
    if (fread(&bitarray, sizeof(t_bitarray), 1, archivo) != 1) {
        perror("Error al leer el archivo de bitmap");
        fclose(archivo);
        return -1;
    }

    fclose(archivo);

    // Buscar el primer bloque libre
    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            return i;  // Retorna el índice del primer bloque libre
        }
    }
    return -1;  // Retorna -1 si no hay bloques libres
}


void setBitmap(int bloque, int block_count) {
    int bitmap_size = (block_count + 7) / 8;

    // Leer el bitmap desde el archivo
    unsigned char* bitmap = (unsigned char*)malloc(bitmap_size);

    if (!bitmap) {
        perror("Error al asignar memoria para leer el bitmap");
        exit(EXIT_FAILURE);
    }

    char* path_archivo = crear_ruta("bitmap.dat");

    FILE* archivo = fopen(path_archivo, "rb+");  // Abrir en modo lectura y escritura
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        free(bitmap);
        exit(EXIT_FAILURE);
    }

    // Leer el bitmap desde el archivo
    if (fread(bitmap, sizeof(unsigned char), bitmap_size, archivo) != bitmap_size) {
        perror("Error al leer el archivo de bitmap");
        free(bitmap);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Marcar el bloque como ocupado
    int byte_index = bloque / 8;
    int bit_index = bloque % 8;
    bitmap[byte_index] |= (1 << bit_index);

    // Volver a escribir el bitmap actualizado en el archivo
    fseek(archivo, 0, SEEK_SET);  // Volver al inicio del archivo
    if (fwrite(bitmap, sizeof(unsigned char), bitmap_size, archivo) != bitmap_size) {
        perror("Error al escribir en el archivo de bitmap");
        free(bitmap);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    free(bitmap);
    fclose(archivo);
}

void cleanBitMap(int bloque, int block_count) {
    int bitmap_size = (block_count + 7) / 8;

    // Leer el bitmap desde el archivo
    unsigned char* bitmap = (unsigned char*)malloc(bitmap_size);

    if (!bitmap) {
        perror("Error al asignar memoria para leer el bitmap");
        exit(EXIT_FAILURE);
    }

    char* path_archivo = crear_ruta("bitmap.dat");

    FILE* archivo = fopen(path_archivo, "rb+");  // Abrir en modo lectura y escritura
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        free(bitmap);
        exit(EXIT_FAILURE);
    }

    // Leer el bitmap desde el archivo
    if (fread(bitmap, sizeof(unsigned char), bitmap_size, archivo) != bitmap_size) {
        perror("Error al leer el archivo de bitmap");
        free(bitmap);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Marcar el bloque como ocupado
    int byte_index = bloque / 8;
    int bit_index = bloque % 8;
    bitmap[byte_index] &= ~(1 << bit_index);

    // Volver a escribir el bitmap actualizado en el archivo
    fseek(archivo, 0, SEEK_SET);  // Volver al inicio del archivo
    if (fwrite(bitmap, sizeof(unsigned char), bitmap_size, archivo) != bitmap_size) {
        perror("Error al escribir en el archivo de bitmap");
        free(bitmap);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    free(bitmap);
    fclose(archivo);
}

bool verificar_bitmap(int bloque, int cant_bloque, int block_count) {
    if (bloque + cant_bloque > block_count) {
        return false;
    }

    int bitmap_size = (block_count + 7) / 8;  // Tamaño del bitmap en bytes
    unsigned char* bitmap = (unsigned char*)malloc(bitmap_size);
    if (!bitmap) {
        perror("Error al asignar memoria para leer el bitmap");
        return -1;
    }

    // Crear la ruta completa para el archivo bitmap.dat
    char* path_archivo = crear_ruta("bitmap.dat");

    FILE* archivo = fopen(path_archivo, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        free(bitmap);
        return -1;
    }

    // Leer el bitmap desde el archivo
    if (fread(bitmap, sizeof(unsigned char), bitmap_size, archivo) != bitmap_size) {
        perror("Error al leer el archivo de bitmap");
        free(bitmap);
        fclose(archivo);
        return -1;
    }

    fclose(archivo);

    for (int i = bloque; i < cant_bloque; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if ((bitmap[byte_index] & (1 << bit_index))) {
            free(bitmap);
            return false;  // Retorna false si hay un 1
        }
    }

    free(bitmap);
    return true;  // Retorna true si no hay 1
}