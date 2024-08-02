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


//extern int block_count;
extern int block_count2;
//extern int block_size;
extern int block_size2;
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
    int bloque_inicial = getBit(block_count2);
    t_metadata metadata;

    if (bloque_inicial == -1) {
        log_info(logger, "NO HAY ESPACIO");
        exit(EXIT_FAILURE);
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
        perror("Error al escribir en el archivo de metadata");
        fclose(archivo);
        exit(EXIT_FAILURE);
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
        log_info(logger,"NO SE ENCONTRO EL ARCHIVO INGRESADO");
        return;
    }
    
    // Marcar el bloque como libre en el bitmap
    int bloque_inicial = FCB->metadata.bloque_inicial;
    int cant_bloques = ((FCB->metadata.tam_archivo + block_size2 -1) / block_size2);
    if(cant_bloques==0){
        cant_bloques=1;
    }
    for (int i=bloque_inicial; i < bloque_inicial+cant_bloques; i++) {
        cleanBitMap(i);
    }

    dictionary_remove_and_destroy(diccionarioFS, nombre, free);

    // Eliminar el archivo de metadata
    if (remove(filepath) != 0) {
        perror("Error al eliminar el archivo de metadata");
        exit(EXIT_FAILURE);
    }
}

//GOD

void truncate_archivo(char* nombre, int tam, int retraso_compactacion,int pid) {
        t_diccionario* FCB;

        int cant_bloques_ingresados=(tam+block_size2-1)/block_size2;  //cantidad de bloques que se desea ocupar

        if(cant_bloques_ingresados>block_count2){ //si es mayor al block count se pasa
            fprintf(stderr, "El tamaño no es valido");
            exit(EXIT_FAILURE);
        }

    FCB =dictionary_get(diccionarioFS,nombre);

    int cant_bloques_arch = (FCB->metadata.tam_archivo+block_size2-1)/block_size2; //cant de bloques que ya ocupa el archivo
    if(cant_bloques_arch==0){
        cant_bloques_arch=1;
    }
    int ultimo_bloque=FCB->metadata.bloque_inicial+cant_bloques_arch-1;



    //SE DESEA ACHICAR EL ARCHIVO
    if (cant_bloques_arch>cant_bloques_ingresados){ 
        int bloques_modificados = cant_bloques_arch-cant_bloques_ingresados; //cuantos bloques se liberan

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
                compactacion_bitmap(espacio,cant_bloques_arch);
                compactacion_metadata(nombre, espacio);
                log_info(logger, "PID: %d - Fin Compactación.",pid);
                truncate_archivo(nombre,tam,retraso_compactacion,pid);
                
                return;
            };
            perror("No hay espacio");
            exit(EXIT_FAILURE);
        }
    }
        
    t_metadata* metadata = (t_metadata*)FCB->map;
    metadata->tam_archivo=tam;   
    FCB->metadata.tam_archivo=tam;
    dictionary_put(diccionarioFS,nombre,FCB);
    msync(FCB->map,sizeof(t_metadata),MS_SYNC);
}

void compactacion_metadata(char* nombre, int espacio){
    char* name;
    t_diccionario* FCB;
    t_list* lista = dictionary_keys(diccionarioFS);
    
    int acumulador_bloques=0;
    for(int i=0;i<list_size(lista);i++){
        name = list_get(lista,i);
        FCB = dictionary_get(diccionarioFS,name);
        
        if(strcmp(nombre,name)){
        
            t_metadata* metadata = (t_metadata*)FCB->map;
            metadata->bloque_inicial=acumulador_bloques;   
            FCB->metadata.bloque_inicial=acumulador_bloques;
            dictionary_put(diccionarioFS,name,FCB);
            msync(FCB->map,sizeof(t_metadata),MS_SYNC);

            int cantidad_bloques=(FCB->metadata.tam_archivo+block_size2-1)/block_size2;
            if(cantidad_bloques==0){
                cantidad_bloques=1;
            }
            acumulador_bloques=acumulador_bloques+cantidad_bloques;
        
        }else{
            t_metadata* metadata = (t_metadata*)FCB->map;
            metadata->bloque_inicial=block_count2-espacio;   
            FCB->metadata.bloque_inicial=block_count2-espacio;
            dictionary_put(diccionarioFS,nombre,FCB);
            msync(FCB->map,sizeof(t_metadata),MS_SYNC);
        }
    }
}


void escribir_archivo(char* nombre, int puntero, int tam, void* dato) {

    // Obtener el FCB del diccionario
    t_diccionario* FCB = dictionary_get(diccionarioFS, nombre);
    if (FCB == NULL) {
        log_info(logger,"ERROR: ARCHIVO NO ENCONTRADO<");
        exit(EXIT_FAILURE);
    }

    if(FCB->metadata.tam_archivo< puntero + tam){
        log_info(logger, "ERROR: NO PUEDE ACCEDER");
        exit(EXIT_FAILURE);
    }

    // Calcular el inicio del archivo
    int inicio_archivo = FCB->metadata.bloque_inicial * block_size2;

    // Realizar la escritura en la memoria mapeada
    memcpy(map_bloque + inicio_archivo + puntero, dato, tam);
    msync(map_bloque, block_count2 * block_size2, MS_SYNC);
}

void* leer_archivo(char* nombre, int puntero, int tam){  
    t_diccionario* FCB = dictionary_get(diccionarioFS,nombre);
    void* dato = calloc(1,tam+1);

    if (FCB == NULL) {
        log_info(logger,"ERROR: ARCHIVO NO ENCONTRADO");
        exit(EXIT_FAILURE);   
    }

    if(FCB->metadata.tam_archivo< puntero + tam){
        log_info(logger, "ERROR: NO PUEDE ACCEDER");
        exit(EXIT_FAILURE);
    }
    mem_hexdump(map_bloque+puntero,tam);
    memcpy(dato, map_bloque+puntero, tam);
    return dato;
}
