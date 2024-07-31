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
extern int block_count;

t_bitarray* crear_bitarray(){
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
    return bitarray;
}

void crear_bitmap() {
    
    t_bitarray* bitarray = crear_bitarray();

    log_info(logger, "Se inicializó el bitarray con un tamaño de %lu", bitarray_get_max_bit(bitarray));

    // chequear si el archivo ya existía
    char* ruta = crear_ruta("bitmap.dat");
    if (access(ruta, F_OK) == 0) return;

    // Crear la ruta completa para el archivo bitmap.dat
    FILE* archivo = crear_archivo_fs("bitmap.dat");

    if ((fwrite(bitarray, sizeof(t_bitarray), 1, archivo)) != 1) {
        perror("Error al escribir en el archivo de bitmap");
        bitarray_destroy(bitarray);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    bitarray_destroy(bitarray);
    fclose(archivo);
}

int getBit() {
    // Crear la ruta completa para el archivo bitmap.dat
    char* path_archivo = crear_ruta("bitmap.dat");
        
    t_bitarray* bitarray = crear_bitarray();

    FILE* archivo = fopen(path_archivo, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        exit(EXIT_FAILURE);
    }


    // Leer el bitmap desde el archivo
    if (fread(bitarray, sizeof(t_bitarray), 1, archivo) != 1) {
        perror("Error al leer el archivo de bitmap");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);

    // Buscar el primer bloque libre
    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            bitarray_destroy(bitarray);  // Destruir el bitarray después de usarlo
            return i;  // Retorna el índice del primer bloque libre
        }
    }

    bitarray_destroy(bitarray);  // Destruir el bitarray después de usarlo
    return -1;  // Retorna -1 si no hay bloques libres
}

void setBitmap(int bloque) {

    char* path_archivo = crear_ruta("bitmap.dat");
        
    t_bitarray* bitarray = crear_bitarray();


    FILE* archivo = fopen(path_archivo, "rb+");  // Abrir en modo lectura y escritura
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        exit(EXIT_FAILURE);
    }

    if (fread(bitarray, sizeof(t_bitarray), 1, archivo) != 1) {
        perror("Error al leer el archivo de bitmap");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }
    
    bitarray_set_bit(bitarray,bloque);

     if (!(fwrite(bitarray, sizeof(t_bitarray), 1, archivo))) {
        perror("Error al escribir en el archivo de bitmap");
        bitarray_destroy(bitarray);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    bitarray_destroy(bitarray);
    fclose(archivo);
}


void cleanBitMap(int bloque) {
        char* path_archivo = crear_ruta("bitmap.dat");
        
    t_bitarray* bitarray = crear_bitarray();


    FILE* archivo = fopen(path_archivo, "rb+");  // Abrir en modo lectura y escritura
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        exit(EXIT_FAILURE);
    }

    if (fread(bitarray, sizeof(t_bitarray), 1, archivo) != 1) {
        perror("Error al leer el archivo de bitmap");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }
    
    bitarray_clean_bit(bitarray,bloque);

     if (!(fwrite(bitarray, sizeof(t_bitarray), 1, archivo))) {
        perror("Error al escribir en el archivo de bitmap");
        bitarray_destroy(bitarray);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    bitarray_destroy(bitarray);
    fclose(archivo);
}


//dado un indice y una cant de bloques comprueba que los siguientes bloques esten en 0
bool verificar_bitmap(int bloque, int cant_bloque) {
    int limite= bloque+cant_bloque; 
    if (bloque + cant_bloque >= block_count) {
        return false;
    }

    // Crear la ruta completa para el archivo bitmap.dat
    char* path_archivo = crear_ruta("bitmap.dat");

    t_bitarray* bitarray=crear_bitarray();

    FILE* archivo = fopen(path_archivo, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo de bitmap");
        bitarray_destroy(bitarray);
        return -1;
    }

    // Leer el bitmap desde el archivo
    if (fread(bitarray, sizeof(t_bitarray), 1, archivo) != 1) {
        perror("Error al leer el archivo de bitmap");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }
    

    fclose(archivo);

    for (int i = bloque + 1 ; i < limite+1; i++) {
        if (bitarray_test_bit(bitarray,i)) {
            bitarray_destroy(bitarray);
            return false;  // Retorna false si hay un 1
        }
    }

    bitarray_destroy(bitarray);
    return true;  // Retorna true si no hay 1
}

