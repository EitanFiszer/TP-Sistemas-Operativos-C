#include "operacionesFS.h"
#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

    void crearArchivo(const char* pathbase, const char* nombre, int block_count) {
        // Crear la ruta completa para el archivo de metadata
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/%s", pathbase, nombre);

        struct{
            int bloque_inicial;
            int tam_archivo;
        }metadata

        metadata.bloque_inicial=getBit(pathbase, block_count);
            if (bloque_inicial == -1) {
                fprintf(stderr, "No hay bloques libres disponibles.\n");
                exit(EXIT_FAILURE);
            }
        
        metadata.tam_archivo=0;

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


