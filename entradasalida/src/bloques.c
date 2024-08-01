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


//extern int block_size;
//extern int block_count;
extern int block_count2;
extern int block_size2;
extern void* map_bloque;

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


