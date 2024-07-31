#include <stdio.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/constants.h>
#include <utils/iniciar.h>
#include <string.h>
#include <pthread.h>
#include "hilos.h"
#include "bitmap.h"
#include "operacionesFS.h"
#include <dirent.h>
#include <utils.h>



extern t_bitarray* bitmap;
extern char* path_base_fs;
extern int block_count;

FILE* crear_archivo_fs(char* nombre) {
    
    char* path_archivo=crear_ruta(nombre);

    FILE* archivo = fopen(path_archivo, "wb+");
    if (!archivo) {
        perror("Error al crear el archivo");
        return NULL;
    }
    return archivo;
}

char* crear_ruta(char* nombre){
    char cwd[256];
    getcwd(cwd, sizeof(cwd));

    char* path_archivo = malloc(strlen(cwd) + strlen(path_base_fs) + strlen(nombre) + 1);
    strcpy(path_archivo, cwd);
    strcat(path_archivo, path_base_fs);
    strcat(path_archivo, nombre);

    return path_archivo;
}


void leerbitmap(){
    for(int i=0; i<block_count; i++){
        printf("%d",bitarray_test_bit(bitmap,i));
    }
    printf("\n");
}

void leerCarpeta(){
    DIR *d;
    struct dirent *dir;
    d = opendir(crear_ruta(""));

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if((string_starts_with(dir->d_name,".")) || (string_ends_with(dir->d_name,"dat"))){
                continue;
            }
                printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
}