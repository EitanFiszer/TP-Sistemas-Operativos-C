#include "operacionesFS.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include "bitmap.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//extern int block_count;
extern int block_count2;
//extern int block_size;
extern int block_size2;
extern t_log* logger;

t_dictionary* diccionarioFS;
t_bitarray* bitmap;

typedef struct {
        int bloque_inicial;
        int tam_archivo;
} t_metadata;

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

//CREA Y CARGA LAS ESTRUCTURAS Y ARCHIVOS NECESARIOS
void inicializar_FS(){
    crear_bitmap();
    bitmap=cargar_bitmap();
    diccionarioFS=dictionary_create();
    cargar_diccionario();
    crearArchivodebloques();
}

//CREA ARCHIVO METADATA
void crear_archivo(char* nombre) {
    t_metadata metadata;
    t_metadata* FCB;
    
    metadata.bloque_inicial = getBit(block_count2);

    if (metadata.bloque_inicial == -1) {
        log_info(logger, "NO HAY ESPACIO");
        exit(EXIT_FAILURE);
    }

    char* ruta = crear_ruta(nombre);
    if (access(ruta, F_OK) == 0){
        log_info(logger, "YA EXISTE EL ARCHIVO");
        return;
    } 

    setBitmap(metadata.bloque_inicial);

    metadata.tam_archivo = 0;

    FCB = malloc(sizeof(t_metadata));
    FCB->bloque_inicial = metadata.bloque_inicial;
    FCB->tam_archivo = metadata.tam_archivo;
    dictionary_put(diccionarioFS,nombre, FCB);

    FILE* archivo= crear_archivo_fs(nombre);
    
    if (fwrite(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
        perror("Error al escribir en el archivo de metadata");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);

}

//BORRA ARCHIVO METADATA
void delete_archivo(char* nombre) {
    t_metadata metadata;

    // Crear la ruta completa para el archivo de metadata
    char* filepath=crear_ruta(nombre);

    // Leer el archivo de metadata
    FILE* archivo = fopen(filepath, "rb");
    if (!archivo) {
        perror("Error al abrir el archivo de metadata");
        exit(EXIT_FAILURE);
    }
  
    if (fread(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
        perror("Error al leer el archivo de metadata");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);

    // Marcar el bloque como libre en el bitmap
    int bloque_inicial = metadata.bloque_inicial;
    int cant_bloques = ((metadata.tam_archivo + block_size2 -1) / block_size2);
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
void truncate_archivo(char* nombre, int tam) {
        t_metadata metadata;
        t_metadata* FCB;

        int cant_bloques_ingresados=(tam+block_size2-1)/block_size2;  //cantidad de bloques que se desea ocupar

        if(cant_bloques_ingresados>block_count2){ //si es mayor al block count se pasa
            fprintf(stderr, "El tamaño no es valido");
            exit(EXIT_FAILURE);
        }

    char* path_archivo= crear_ruta(nombre);

    FILE* archivo= fopen(path_archivo,"rb+");
        if (!archivo) {
        perror("Error al abrir el archivo de metadata");
        exit(EXIT_FAILURE);
    }

    if (fread(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
        perror("Error al leer el archivo de metadata");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    int cant_bloques_arch = (metadata.tam_archivo+block_size2-1)/block_size2; //cant de bloques que ya ocupa el archivo
    if(cant_bloques_arch==0){
        cant_bloques_arch=1;
    }
    int ultimo_bloque=metadata.bloque_inicial+cant_bloques_arch-1;



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
                compactacion_bitmap(espacio,cant_bloques_arch);
                compactacion_metadata(nombre, espacio);
                log_info(logger, "COMPACTACION");
                
                return;
            };
            perror("No hay espacio");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }
    }
        metadata.tam_archivo=tam;
        
        FCB = malloc(sizeof(t_metadata));
        FCB->bloque_inicial = metadata.bloque_inicial;
        FCB->tam_archivo = metadata.tam_archivo;        
        
        dictionary_put(diccionarioFS,nombre,FCB);

        fseek(archivo, 0, SEEK_SET);

        
        if (fwrite(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
            perror("Error al escribir en el archivo de metadata");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }

    fclose(archivo);
}

void compactacion_metadata(char* nombre, int espacio){
    DIR *d;
    struct dirent *dir;
    d = opendir(crear_ruta(""));
    t_metadata* FCB;

    if (d) {
        int acumulador_bloques=0;
        while ((dir = readdir(d)) != NULL) {
            if((string_starts_with(dir->d_name,".")) || (string_ends_with(dir->d_name,"dat"))){
                continue;
            }
            FCB=dictionary_get(diccionarioFS,dir->d_name);
            if(strcmp(nombre,dir->d_name)){
                FCB->bloque_inicial=acumulador_bloques;
                dictionary_put(diccionarioFS,dir->d_name,FCB);

                int cantidad_bloques=(FCB->tam_archivo+block_size2-1)/block_size2;
                if(cantidad_bloques==0){
                    cantidad_bloques=1;
                }
                acumulador_bloques=acumulador_bloques+cantidad_bloques;
            }else{
                FCB->bloque_inicial=block_count2-espacio;
                dictionary_put(diccionarioFS,dir->d_name,FCB);
            }
        }
        closedir(d);
    }

}


