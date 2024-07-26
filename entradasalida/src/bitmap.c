#include "bitmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <commons/bitarray.h>

void crearBitmap(const char *pathbase, int block_count) {
    
    int bitmap_size = (block_count + 7) / 8; // Cada bit representa un bloque, +7 para redondear hacia arriba

    // Crear e inicializar el bitmap con todos los bits en 0 (todos los bloques libres)
    unsigned char *bitmap = (unsigned char *)calloc(bitmap_size, sizeof(unsigned char));
    if (!bitmap) {
        perror("Error al asignar memoria para el bitmap");
        exit(EXIT_FAILURE);
    }

    // Crear la ruta completa para el archivo bitmap.dat
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/bitmap.dat", pathbase);

    FILE *archivo = fopen(filepath, "wb");
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        free(bitmap);
        exit(EXIT_FAILURE);
    }

    if (fwrite(bitmap, sizeof(unsigned char), bitmap_size, archivo) != bitmap_size) {
        perror("Error al escribir en el archivo de bitmap");
        free(bitmap);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    free(bitmap);
    fclose(archivo);
}

int getBit(const char *pathbase, int block_count) {
    int bitmap_size = (block_count + 7) / 8; // Tamaño del bitmap en bytes
    unsigned char *bitmap = (unsigned char *)malloc(bitmap_size);
    if (!bitmap) {
        perror("Error al asignar memoria para leer el bitmap");
        return -1;
    }

    // Crear la ruta completa para el archivo bitmap.dat
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/bitmap.dat", pathbase);

    FILE* archivo = fopen(filepath, "rb");
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

    // Buscar el primer bloque libre
    for (int i = 0; i < block_count; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;
        if (!(bitmap[byte_index] & (1 << bit_index))) {
            free(bitmap);
            return i; // Retorna el índice del primer bloque libre
        }
    }

    free(bitmap);
    return -1; // Retorna -1 si no hay bloques libres
}

void setBitmap()