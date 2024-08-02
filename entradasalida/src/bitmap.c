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
//extern int block_count;
extern int block_count2;

//CREA EL BITARRAY
t_bitarray* crear_bitarray(){

   char data[(int)ceil(block_count2/8)];
   t_bitarray* bitarray = bitarray_create_with_mode(data, sizeof(data),LSB_FIRST);
    for(int i=0;i<block_count2;i++){
        bitarray_clean_bit(bitarray,i);
        log_info(logger,"%d",bitarray_test_bit(bitarray,i));
    }
    log_info(logger, "\n");

    return bitarray;
}

//CREA EL ARCHIVO BITMAP.DAT SINO EXISTE
void crear_bitmap() {
    char* ruta=crear_ruta("bitmap.dat");

    // chequear si el archivo ya existía
    if (access(ruta, F_OK) == 0){
        return;
    } 

    t_bitarray* bitarray = crear_bitarray();
    FILE* archivo = crear_archivo_fs("bitmap.dat");

    ftruncate(fileno(archivo),bitarray->size);
/*
    if ((fwrite(bitarray->bitarray, 1, bitarray->size, archivo)) != bitarray->size) {
        perror("Error al escribir en el archivo de bitmap");
        bitarray_destroy(bitarray);
        fclose(archivo);
        exit(EXIT_FAILURE);
    }
*/

    fseek(archivo, 0, SEEK_SET);

    fclose(archivo);
    bitarray_destroy(bitarray);
}

//CARGA EL BITMAP EN DISCO A PARTIR DEL ARCHIVO BITMAP.DAT
t_bitarray* cargar_bitmap() {

    int fd = open(crear_ruta("bitmap.dat"), O_CREAT | O_RDWR, 0664);

    ftruncate(fd,(int)ceil(block_count2/8));

    void* bitmap = mmap(NULL, (int)ceil(block_count2/8), PROT_READ|PROT_WRITE, MAP_SHARED, fd ,0);

	t_bitarray* bitarray = bitarray_create_with_mode((char*) bitmap, (int)ceil(block_count2/8), LSB_FIRST);
//    bitarray_set_bit(bitarray,0);
    close(fd);
    return bitarray;
}


//BUSCA EL PRIMER 0, SINO HAY DEVUELVE -1
int getBit() {
        
    for (int i = 0; i < block_count2; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            return i; 
        }
    }
    return -1;
}

//PONE EN 1 segun el indice
void setBitmap(int bloque) {
            
    bitarray_set_bit(bitmap,bloque);
    msync(bitmap,bitmap->size,MS_SYNC);

}

//PONE EN 0 segun el indice
void cleanBitMap(int bloque) {
    bitarray_clean_bit(bitmap,bloque);
    msync(bitmap,bitmap->size,MS_SYNC);
}


//A PARTIR DE UN INDICE Y UNA CANTIDAD DE BLOQUES VERIFICA QUE LOS SIGUIENTES ESTEN EN 0
bool verificar_bitmap(int bloque, int cant_bloque) {
    int limite= bloque+cant_bloque; 

    if (bloque + cant_bloque >= block_count2) { //SI SE PASA DEL BITARRAY
        return false;
    }

    for (int i = bloque + 1 ; i < limite+1; i++) {
        if (bitarray_test_bit(bitmap,i)) {
            return false; 
        }
    }
    return true; 
}

void cleanALL(){
    for(int i=0;i<block_count2;i++){
        cleanBitMap(i);
    }
}

void compactacion_bitmap(int espacio, int cant_bloques_arch){
    cleanALL();
    for(int i=0; i<(block_count2-espacio)+cant_bloques_arch; i++){
        setBitmap(i);
    }
}

void leerbitmap(){
    for(int i=0; i<block_count2; i++){
        printf("%d",bitarray_test_bit(bitmap,i));
    }
    printf("\n");
}
