#include "operacionesFS.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/memory.h>
#include "bitmap.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <bloques.h>
#include <sys/mman.h>
#include <diccionario.h>
#include <utils/constants.h>


extern int block_count;
extern int block_size;
extern t_log* logger;

t_dictionary* diccionarioFS;
t_bitarray* bitmap;
void* map_bloque;




//CREA Y CARGA LAS ESTRUCTURAS Y ARCHIVOS NECESARIOS
void inicializar_FS(){
    crear_bitmap();
    bitmap=cargar_bitmap();
    diccionarioFS=incializar_el_diccionario();
    crearArchivodebloques();
    map_bloque=cargar_bloques();
}

//CREA ARCHIVO METADATA
void crear_archivo(char* nombre) {    
    int bloque_inicial = getBit(block_count);
    t_metadata metadata;

    if (bloque_inicial == -1) {
        log_info(logger, "NO HAY ESPACIO");
        return;
    }

    char* ruta = crear_ruta(nombre);
    if (access(ruta, F_OK) == 0){
        log_info(logger, "YA EXISTE EL ARCHIVO");
        return;
    } 

    setBitmap(bloque_inicial);
    
    
    metadata.bloque_inicial=bloque_inicial;
    metadata.tam_archivo=0;
    FILE* archivo= crear_archivo_fs(nombre);
    if  (fwrite(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
        log_error(logger,"Error al escribir en el archivo de metadata");
        fclose(archivo);
        return;
    }
    fclose(archivo);

    cargar_diccionario_nuevo(nombre, bloque_inicial);
}

//BORRA ARCHIVO METADATA
void delete_archivo(char* nombre) {

    // Crear la ruta completa para el archivo de metadata
    char* filepath=crear_ruta(nombre);

    t_diccionario* FCB = dictionary_get(diccionarioFS,nombre);
    if(!FCB){
        log_error(logger,"NO SE ENCONTRO EL ARCHIVO INGRESADO");
        return;
    }
    
    // Marcar el bloque como libre en el bitmap
    int bloque_inicial = FCB->map->bloque_inicial;
    int cant_bloques = ((FCB->map->tam_archivo + block_size -1) / block_size);
    if(cant_bloques==0){
        cant_bloques=1;
    }
    for (int i=bloque_inicial; i < bloque_inicial+cant_bloques; i++) {
        cleanBitMap(i);
    }
    void* borrar = calloc(1,cant_bloques*block_size);

    memcpy(map_bloque+FCB->map->bloque_inicial*block_size , borrar , cant_bloques * block_size);
    msync(map_bloque,block_count*block_size,MS_SYNC);

    dictionary_remove_and_destroy(diccionarioFS, nombre, free);
    free(borrar);
    

    // Eliminar el archivo de metadata
    if (remove(filepath) != 0) {
        log_error(logger,"Error al eliminar el archivo de metadata");
        return;
    }
    return;
}

//GOD

void truncate_archivo(char* nombre, int tam, int retraso_compactacion,int pid) {
    t_diccionario* FCB;

    int cant_bloques_ingresados=(tam+block_size-1)/block_size;  //cantidad de bloques que se desea ocupar

    if(cant_bloques_ingresados>block_count){ //si es mayor al block count se pasa
        log_error(logger, "El tamaño no es valido");
        return;
    }

    FCB =dictionary_get(diccionarioFS,nombre);
    if(!FCB){
        log_error(logger, "NO SE ENCONTRO EL ARCHIVO");
        return;
    }
    int cant_bloques_arch = (FCB->map->tam_archivo+block_size-1)/block_size; //cant de bloques que ya ocupa el archivo
    if(cant_bloques_arch==0){
        cant_bloques_arch=1;
    }
    int ultimo_bloque=FCB->map->bloque_inicial+cant_bloques_arch-1;

    //SE DESEA ACHICAR EL ARCHIVO
    if (cant_bloques_arch>cant_bloques_ingresados){ 
        int bloques_modificados = cant_bloques_arch-cant_bloques_ingresados; //cuantos bloques se liberan
        void* borrar = calloc(1,bloques_modificados*block_size);
        memcpy(map_bloque+(block_size*FCB->map->bloque_inicial)+(tam), borrar , bloques_modificados * block_size);
        msync(map_bloque,block_count*block_size,MS_SYNC);        

        free(borrar);
        for(int i=0; i<bloques_modificados; i++){
            cleanBitMap(ultimo_bloque);
            ultimo_bloque=ultimo_bloque-1;
        }
    }
    
    //SE DESEA AGRANDAR EL ARCHIVO
    if(cant_bloques_arch<cant_bloques_ingresados){ 
        int bloques_modificados=cant_bloques_ingresados-cant_bloques_arch; //cuantos bloques se ocupan
        if(verificar_bitmap(ultimo_bloque,bloques_modificados)){
            for(int i=1; i<bloques_modificados+1;i++){
                setBitmap(ultimo_bloque+i);
            }
        }else{
            int espacio=espacioLIbre(nombre, cant_bloques_arch);
            if(espacio>=cant_bloques_ingresados){
                log_info(logger, "PID: %d - Inicio Compactación.",pid);
                sleep(retraso_compactacion/1000);
                compactacion_bloques(nombre);
                log_info(logger, "PID: %d - Fin Compactación.",pid);
                truncate_archivo(nombre,tam,retraso_compactacion,pid);
                return;
            };
            perror("No hay espacio");
            exit(EXIT_FAILURE);
        }
    }
        
    FCB->map->tam_archivo=tam;
    msync(FCB->map,sizeof(t_metadata),MS_SYNC);
}


void escribir_archivo(char* nombre, int puntero, int tam, void* dato) {

    // Obtener el FCB del diccionario
    t_diccionario* FCB = dictionary_get(diccionarioFS, nombre);
    if (FCB == NULL) {
        log_info(logger,"ERROR: ARCHIVO NO ENCONTRADO<");
        exit(EXIT_FAILURE);
    }

    if(FCB->map->tam_archivo< puntero + tam){
        log_info(logger, "ERROR: NO PUEDE ACCEDER");
        exit(EXIT_FAILURE);
    }

    // Calcular el inicio del archivo
    int inicio_archivo = FCB->map->bloque_inicial * block_size;

    // Realizar la escritura en la memoria mapeada
    memcpy(map_bloque + inicio_archivo + puntero, dato, strlen(dato));
    msync(map_bloque, block_count * block_size, MS_SYNC);
}

void* leer_archivo(char* nombre, int puntero, int tam){  
    t_diccionario* FCB = dictionary_get(diccionarioFS,nombre);
    void* dato = calloc(1,tam);

    if (FCB == NULL) {
        log_info(logger,"ERROR: ARCHIVO NO ENCONTRADO");
        exit(EXIT_FAILURE);   
    }

    if(FCB->map->tam_archivo< puntero + tam){
        log_info(logger, "ERROR: NO PUEDE ACCEDER");
        exit(EXIT_FAILURE);
    }

    mem_hexdump(map_bloque+puntero,tam);
    memcpy(dato, map_bloque+puntero, tam);
    return dato;
}


void liberarFS(){
    t_list* lista= dictionary_elements(diccionarioFS);
    munmap(map_bloque,block_count*block_size);
    munmap(bitmap,bitmap->size);

    for(int i=0;i<list_size(lista);i++){
        t_diccionario* FCB = list_get(lista,i);
        munmap(FCB->map,sizeof(t_metadata));
    }

    return;
}


