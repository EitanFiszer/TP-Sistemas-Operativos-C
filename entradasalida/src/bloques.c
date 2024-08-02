#include "bloques.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "bitmap.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <diccionario.h>
#include <utils/constants.h>
#include "utils.h"
#include <stdlib.h>
#include "bitmap.h"
#include <math.h>

//extern int block_size;
//extern int block_count;
extern int block_count2;
extern int block_size2;
extern void* map_bloque;
extern t_dictionary* diccionarioFS;
extern t_bitarray* bitmap;

//CREA EN CASO DE QUE NO EXISTE EL BLOQUES.DAT
void crearArchivodebloques() {
    size_t tamano_total = block_size2 * block_count2;

    // chequear si el archivo ya existía
    char* ruta = crear_ruta("bloques.dat");
    if (access(ruta, F_OK) == 0) return;
    
    FILE* archivo = crear_archivo_fs("bloques.dat");

    // Establecer el tamaño del archivo
    if (fseek(archivo, tamano_total - 1, SEEK_SET) != 0) {
        perror("Error al ajustar el tamaño del archivo");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    // Escribir un byte nulo al final para establecer el tamaño
    if (fwrite("", 1, 1, archivo) != 1) {
        perror("Error al escribir el byte final");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);
}

void* cargar_bloques(){
    
    int size = block_count2*block_size2;
    int fd = open(crear_ruta("bloques.dat"), O_CREAT | O_RDWR, 0664);

    ftruncate(fd,size);

    void* map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd ,0);
    
    close(fd);
    return map;
}


void compactacion_bloques(char* nombre){
    t_diccionario* FCB=dictionary_get(diccionarioFS,nombre);

    int total_bloques=(int)ceil(FCB->map->tam_archivo/block_size2);
    if(!total_bloques){
        total_bloques=1;
    }

    for(int i=FCB->map->bloque_inicial; i<total_bloques+FCB->map->bloque_inicial; i++){
        cleanBitMap(i);
        leerbitmap();
    }

    void* buffer=calloc(1,block_size2*block_count2);
    memcpy(buffer,map_bloque+(block_size2*FCB->map->bloque_inicial),total_bloques*block_size2);

    int bit=0;
    while(bit<block_count2){
        bit=getBit();
        
        leerbitmap();

        int inicio_del_hueco=bit;
        int contiguo_size=0;

        while(!bitarray_test_bit(bitmap,bit) && bit!=block_count2){
            bit++;
        }

        if(bit==block_count2){
            break;
        }

        int inicio_del_contiguo=bit;
        while(bitarray_test_bit(bitmap,bit)){
            contiguo_size++;
            bit++;
        }
        if(!contiguo_size){
            break;
        }

        int j = 0;
        while(j<contiguo_size){
            t_diccionario* fcb_cont=getFCBxInicio(inicio_del_contiguo+j);
             if (fcb_cont == NULL || fcb_cont->map == NULL) {
                perror("Error: fcb_cont o fcb_cont->map es NULL");
                exit(EXIT_FAILURE);
            }
            int cantidad_bloques_fs=(int)ceil(fcb_cont->map->tam_archivo/block_size2);
            if(!cantidad_bloques_fs){
                cantidad_bloques_fs=1;
            }
            fcb_cont->map->bloque_inicial=inicio_del_hueco+j;
            msync(fcb_cont->map,sizeof(t_metadata),MS_SYNC);
            j=j+cantidad_bloques_fs;
        }
        memcpy(map_bloque+inicio_del_hueco*block_size2,map_bloque+inicio_del_contiguo*block_size2,contiguo_size*block_size2);
        for(int k=inicio_del_hueco;k<inicio_del_hueco+contiguo_size;k++){
            bitarray_set_bit(bitmap,k);
        }
        for(int k=inicio_del_hueco+contiguo_size;k<bit;k++){
            bitarray_clean_bit(bitmap,k);
        }
    }
    int ultimobit=getBit();

    FCB->map->bloque_inicial=ultimobit;
    msync(FCB->map,sizeof(t_metadata),MS_SYNC);
    
    int primerbit=getBit();
    memcpy(map_bloque+ultimobit*block_size2,buffer,(block_count2-primerbit)*block_size2);
    free(buffer);

    for(int i=ultimobit;i<total_bloques;i++){
        bitarray_set_bit(bitmap,i);
    }

    msync(bitmap,block_count2,MS_SYNC);
    msync(map_bloque,block_count2*block_size2,MS_SYNC);
}



