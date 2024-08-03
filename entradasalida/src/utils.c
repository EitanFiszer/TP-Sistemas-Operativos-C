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


extern char* path_base_fs;

//CREA ARCHIVO
FILE* crear_archivo_fs(char* nombre) {
    
    char* path_archivo=crear_ruta(nombre);

    FILE* archivo = fopen(path_archivo, "wb+");
    if (!archivo) {
        perror("Error al crear el archivo");
        return NULL;
    }
    return archivo;
}

//LINKEA EL PATH_BASE CON EL NOMBRE DEL ARCHIVO
char* crear_ruta(char* nombre){
    char cwd[256];
    getcwd(cwd, sizeof(cwd));

    char* path_archivo = malloc(strlen(cwd) + strlen(path_base_fs) + strlen(nombre) + 1);
    strcpy(path_archivo, cwd);
    strcat(path_archivo, path_base_fs);
    strcat(path_archivo, nombre);

    return path_archivo;
}
