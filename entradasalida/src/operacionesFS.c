#include "operacionesFS.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    void crear_archivo(const char* pathbase, const char* nombre, int block_count, int block_size) {
        // Crear la ruta completa para el archivo de metadata
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", pathbase, nombre);

        struct{
            int bloque_inicial;
            int tam_archivo;
        }metadata;

        metadata.bloque_inicial=getBit(pathbase, block_count);
            if (metadata.bloque_inicial == -1) {
                fprintf(stderr, "No hay bloques libres disponibles.\n");
                exit(EXIT_FAILURE);
            }
        setBitmap(pathbase, block_count, metadata.bloque_inicial);
        
        metadata.tam_archivo=block_size;

        FILE* archivo = fopen(filepath, "wb");
        if (!archivo) {
            perror("Error al abrir el archivo de metadata");
            exit(EXIT_FAILURE);
        }

        if (fwrite(&metadata, sizeof(metadata), 1, archivo) != 1) {
            perror("Error al escribir en el archivo de metadata");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }  

    fclose(archivo);

    }   

    void delete_archivo(const char* pathbase, const char* nombre, int block_count, int block_size) {
    // Crear la ruta completa para el archivo de metadata
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", pathbase, nombre);

        // Leer el archivo de metadata
        FILE* archivo = fopen(filepath, "rb");
        if (!archivo) {
            perror("Error al abrir el archivo de metadata");
            exit(EXIT_FAILURE);
        }

        // Leer la metadata del archivo
        struct {
            int bloque_inicial;
            int tam_archivo;
        } metadata;

        if (fread(&metadata, sizeof(metadata), 1, archivo) != 1) {
            perror("Error al leer el archivo de metadata");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }

        fclose(archivo);

        // Marcar el bloque como libre en el bitmap
        int bloque_inicial = metadata.bloque_inicial;
        int cant_bloques = metadata.tam_archivo/block_size; 
        
        for(int i=0;i<cant_bloques;i++){
            int bloque = bloque_inicial+i;
            cleanBitMap(pathbase, bloque, block_count);
        }

        // Eliminar el archivo de metadata
        if (remove(filepath) != 0) {
            perror("Error al eliminar el archivo de metadata");
            exit(EXIT_FAILURE);
        }
    }


