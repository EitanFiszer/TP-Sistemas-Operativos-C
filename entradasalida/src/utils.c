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
//extern char* path_base_fs;
extern char* path_base2;
//extern int block_count;
extern int block_count2;
//extern int block_size;
extern int block_size2;
extern t_dictionary* diccionarioFS;


typedef struct {
        int bloque_inicial;
        int tam_archivo;
} t_metadata;

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

    char* path_archivo = malloc(strlen(cwd) + strlen(path_base2) + strlen(nombre) + 1);
    strcpy(path_archivo, cwd);
    strcat(path_archivo, path_base2);
    strcat(path_archivo, nombre);

    return path_archivo;
}

void leerbitmap(){
    for(int i=0; i<block_count2; i++){
        printf("%d",bitarray_test_bit(bitmap,i));
    }
    printf("\n");
}

//CARGA EN EL DICCIONARIO TODOS LOS ARCHIVOS DEL DIRECTORIO -{("."),".dat"}
void cargar_diccionario(){
    DIR *d;
    struct dirent *dir;
    d = opendir(crear_ruta(""));
    t_metadata* FCB;
    t_metadata meta;

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if((string_starts_with(dir->d_name,".")) || (string_ends_with(dir->d_name,"dat"))){
                continue;
            }

            FILE* archivo=fopen((crear_ruta(dir->d_name)),"rb");
            if (fread(&meta, sizeof(t_metadata), 1, archivo) != 1) {
                perror("Error al leer el archivo de metadata");
                fclose(archivo);
                exit(EXIT_FAILURE);
            }

            FCB = malloc(sizeof(t_metadata));
            FCB->bloque_inicial =meta.bloque_inicial;
            FCB->tam_archivo = meta.tam_archivo;
            dictionary_put(diccionarioFS,dir->d_name,FCB);
        }
        closedir(d);
    }
}


//CALCULA LOS BLOQUES LIBRES QUE TIENE UN ARCHIVO PARA TOMAR
int espacioLIbre(char* nombres, int bloques_delArchivo){
    int acumulador_bloques=0;
    t_list* lista = dictionary_elements(diccionarioFS);
    t_metadata* FCB;

    for(int i=0; i<lista->elements_count; i++){
        FCB=lista->head->data;
        int cantidad_bloques;

        if(FCB->tam_archivo==0){
            cantidad_bloques=1;
        }else{
            cantidad_bloques=(FCB->tam_archivo+block_size2-1)/block_size2;
        }

        acumulador_bloques=acumulador_bloques + cantidad_bloques;
        lista->head=lista->head->next;
    }

    int espacio= block_count2 - acumulador_bloques + bloques_delArchivo; //hay que restarle los que ya ocupa el propio archivo
    return espacio; 
}

void leerDiccionario(){
    t_metadata* FCB;
    t_list* lista = dictionary_keys(diccionarioFS);
    char* nombre;
    for(int i=0; i<list_size(lista); i++){
        nombre=list_get(lista,i);
        FCB=dictionary_get(diccionarioFS, nombre);
        log_info(logger,"%s",nombre);
        log_info(logger,"%d",FCB->bloque_inicial);
        log_info(logger,"%d",FCB->tam_archivo);
    }
}

