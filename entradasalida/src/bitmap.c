#include "bitmap.h"
#include <commons/bitarray.h>
#include <commons/log.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>
#include "utils.h"
#include <fcntl.h>
#include <sys/mman.h>

extern t_log* logger;
extern t_bitarray* bitmap;
extern int block_count;

t_bitarray* crear_bitarray(){

   char data[(int)ceil(block_count/8)];
   t_bitarray* bitarray = bitarray_create_with_mode(data, sizeof(data),LSB_FIRST);

    return bitarray;
}

void crear_bitmap() {

    // chequear si el archivo ya existía
    char* ruta = crear_ruta("bitmap.dat");
    if (access(ruta, F_OK) == 0){
        log_info(logger, "YA EXISTE EL BITMAP");
        return;
    } 

    t_bitarray* bitarray = crear_bitarray();
    log_info(logger, "Se inicializó el bitarray con un tamaño de %lu", bitarray_get_max_bit(bitarray));

    // Crear la ruta completa para el archivo bitmap.dat
    FILE* archivo = crear_archivo_fs("bitmap.dat");

    if ((fwrite(bitarray->bitarray, 1, bitarray->size, archivo)) != 1) {
        perror("Error al escribir en el archivo de bitmap");
        bitarray_destroy(bitarray);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    bitarray_destroy(bitarray);
    fclose(archivo);
}

t_bitarray* cargar_bitmap() {

    // Crear la ruta completa para el archivo bitmap.dat
    //FILE* archivo = fopen(crear_ruta("bitmap.dat"),"rb");
    int fd = open(crear_ruta("bitmap.dat"), O_CREAT | O_RDWR, 0664);

    ftruncate(fd,block_count);

    void* bitmap = mmap(NULL, block_count, PROT_READ|PROT_WRITE, MAP_SHARED, fd ,0);

	t_bitarray* bitarray = bitarray_create_with_mode((char*) bitmap, block_count/8, LSB_FIRST);

    //munmap(bitmap, block_count);
    close(fd);
    return bitarray;
}

int getBit() {
        
    // Buscar el primer bloque libre
    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            return i; 
        }
    }
    return -1;  // Retorna -1 si no hay bloques libres
}

void setBitmap(int bloque) {
            
    bitarray_set_bit(bitmap,bloque);
    msync(bitmap,bitmap->size,MS_SYNC);

}


void cleanBitMap(int bloque) {
    bitarray_clean_bit(bitmap,bloque);
    msync(bitmap,bitmap->size,MS_SYNC);
}


//dado un indice y una cant de bloques comprueba que los siguientes bloques esten en 0
bool verificar_bitmap(int bloque, int cant_bloque) {
    int limite= bloque+cant_bloque; 
    if (bloque + cant_bloque >= block_count) {
        return false;
    }

    for (int i = bloque + 1 ; i < limite+1; i++) {
        if (bitarray_test_bit(bitmap,i)) {
            return false;  // Retorna false si hay un 1
        }
    }
    return true;  // Retorna true si no hay 1
}

