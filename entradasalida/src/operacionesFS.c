#include "operacionesFS.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "utils.h"

typedef struct {
        int bloque_inicial;
        int tam_archivo;
} t_metadata;

void crear_archivo(const char* nombre, int block_count, int block_size) {
    t_metadata metadata;

    metadata.bloque_inicial = getBit(block_count);

    if (metadata.bloque_inicial == -1) {
        fprintf(stderr, "No hay bloques libres disponibles.\n");
        exit(EXIT_FAILURE);
    }
    setBitmap(block_count, metadata.bloque_inicial);

    metadata.tam_archivo = block_size;

    FILE* archivo= crear_archivo_fs(nombre);
  
    if (fwrite(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
        perror("Error al escribir en el archivo de metadata");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);
}

void delete_archivo(const char* nombre, int block_count, int block_size) {
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
    int cant_bloques = metadata.tam_archivo / block_size;

    for (int i = 0; i < cant_bloques; i++) {
        int bloque = bloque_inicial + i;
        cleanBitMap(bloque, block_count);
    }

    // Eliminar el archivo de metadata
    if (remove(filepath) != 0) {
        perror("Error al eliminar el archivo de metadata");
        exit(EXIT_FAILURE);
    }
}

void truncate_archivo(const char* nombre, int block_count, int block_size, int tam) {
        t_metadata metadata;

        int cant_bloques=tam/block_size;

        if(cant_bloques>block_count){
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

    int cant_bloques_arch= metadata.tam_archivo/block_count;

    if (metadata.tam_archivo>=tam){
        int bloques_modificados= cant_bloques_arch-cant_bloques;

        for(int i=0; i<bloques_modificados; i++){
            cleanBitMap(i+ metadata.bloque_inicial+ bloques_modificados ,block_count);
        }
        
    }else{
        if(verificar_bitmap(metadata.bloque_inicial,cant_bloques,block_count)){
            int bloques_modificados=cant_bloques-cant_bloques_arch;
            for(int i=0; i<bloques_modificados;i++){
                setBitmap(i+metadata.bloque_inicial+bloques_modificados, block_count);
            }
        }else{
            perror("No hay espacio");
            exit(EXIT_FAILURE);
        }

    }
        metadata.tam_archivo=tam;
        if(fwrite(&metadata,sizeof(t_metadata),1, archivo)!= 1){
            perror("Error al escribir en el archivo de metadata");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }
}


    

