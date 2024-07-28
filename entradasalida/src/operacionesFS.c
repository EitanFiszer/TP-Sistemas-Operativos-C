#include "operacionesFS.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "utils.h"

extern int block_count;
extern int block_size;

typedef struct {
        int bloque_inicial;
        int tam_archivo;
} t_metadata;

void crear_archivo(const char* nombre) {
    t_metadata metadata;

    metadata.bloque_inicial = getBit(block_count);

    if (metadata.bloque_inicial == -1) {
        fprintf(stderr, "No hay bloques libres disponibles.\n");
        exit(EXIT_FAILURE);
    }

    setBitmap(metadata.bloque_inicial);

    metadata.tam_archivo = 0;

    FILE* archivo= crear_archivo_fs(nombre);
  
    if (fwrite(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
        perror("Error al escribir en el archivo de metadata");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    fclose(archivo);
}

void delete_archivo(const char* nombre) {
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
        cleanBitMap(bloque);
    }

    // Eliminar el archivo de metadata
    if (remove(filepath) != 0) {
        perror("Error al eliminar el archivo de metadata");
        exit(EXIT_FAILURE);
    }
}

void truncate_archivo(const char* nombre, int tam) {
        t_metadata metadata;

        int cant_bloques_ingresados=(tam+block_size-1)/block_size;  //cantidad de bloques que se desea ocupar

        if(cant_bloques_ingresados>block_count){ //si es mayor al block count se pasa
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

    int cant_bloques_arch = (metadata.tam_archivo+block_size-1)/block_size; //cant de bloques que ya ocupa el archivo
    if(cant_bloques_arch==0){
        cant_bloques_arch=1;
    }
    int ultimo_bloque=metadata.bloque_inicial+cant_bloques_arch-1;

    if (cant_bloques_arch>cant_bloques_ingresados){ //Se desea achicar el archivo
        int bloques_modificados = cant_bloques_arch-cant_bloques_ingresados; //cuantos bloques se liberan

        for(int i=0; i<bloques_modificados; i++){
            cleanBitMap(ultimo_bloque);
            ultimo_bloque=ultimo_bloque-1;
        }

    }if(cant_bloques_arch<cant_bloques_ingresados){ //Se desea agrandar el archivo
        int bloques_modificados=cant_bloques_ingresados-cant_bloques_arch; //cuantos bloques se ocupan
        if(verificar_bitmap(ultimo_bloque,bloques_modificados)){
            for(int i=1; i<bloques_modificados+1;i++){
                setBitmap(ultimo_bloque+i);
            }
        }else{
            perror("No hay espacio");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }
    }
        metadata.tam_archivo=tam;
        fseek(archivo, 0, SEEK_SET);

        
        if (fwrite(&metadata, sizeof(t_metadata), 1, archivo) != 1) {
            perror("Error al escribir en el archivo de metadata");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }

    fclose(archivo);
}




