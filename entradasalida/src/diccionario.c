#include <diccionario.h>
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
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/hello.h>
#include <utils/server.h>
#include <utils/client.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <utils/constants.h>
#include <utils/iniciar.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utils/constants.h>

extern t_log* logger;
//extern char* path_base_fs;
extern char* path_base2;
//extern int block_count;
extern int block_count2;
//extern int block_size;
extern int block_size2;
extern t_dictionary* diccionarioFS;


//LEE DICCIONARIO
void leerDiccionario(){
    t_diccionario* FCB;
    t_list* lista = dictionary_keys(diccionarioFS);
    char* nombre;
    for(int i=0; i<list_size(lista); i++){
        nombre=list_get(lista,i);
        FCB=dictionary_get(diccionarioFS, nombre);
        log_info(logger,"%s",nombre);
        log_info(logger,"%d",FCB->map->bloque_inicial);
        log_info(logger,"%d",FCB->map->tam_archivo);
    }
}

//CALCULA LOS BLOQUES LIBRES QUE TIENE UN ARCHIVO PARA TOMAR
int espacioLIbre(char* nombres, int bloques_delArchivo){
    int acumulador_bloques=0;
    t_list* lista = dictionary_elements(diccionarioFS);
    t_diccionario* FCB;

    for(int i=0; i<lista->elements_count; i++){
        FCB=lista->head->data;
        int cantidad_bloques;

        if(FCB->map->tam_archivo==0){
            cantidad_bloques=1;
        }else{
            cantidad_bloques=(FCB->map->tam_archivo+block_size2-1)/block_size2;
        }

        acumulador_bloques=acumulador_bloques + cantidad_bloques;
        lista->head=lista->head->next;
    }

    int espacio= block_count2 - acumulador_bloques + bloques_delArchivo; //hay que restarle los que ya ocupa el propio archivo
    return espacio; 
}


//INICIALIZA EL DICCIONARIO Y MAPEA LOS ARCHIVOS QUE YA SE ENCUENTRAN EN EL DIRECTORIO 
t_dictionary* incializar_el_diccionario() {
    
    t_dictionary* diccionario = dictionary_create();
    t_diccionario* FCB;
        
    DIR *d;
    struct dirent *dir;
    d = opendir(crear_ruta(""));

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (string_starts_with(dir->d_name, ".") || string_ends_with(dir->d_name, "dat")) {
                continue;
            }

            char* path = crear_ruta(dir->d_name);
            
            int fd = open(path, O_CREAT | O_RDWR, 0664);

            ftruncate(fd, sizeof(t_metadata));

            void* map = mmap(NULL, sizeof(t_metadata), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

            FCB = malloc(sizeof(t_diccionario));
                        
            FCB->map = map;
            FCB->size = sizeof(t_metadata);
            FCB->fd = fd;

            dictionary_put(diccionario, dir->d_name, FCB);

            close(fd);
        }
        closedir(d);
    }
    return diccionario;
}

void cargar_diccionario_nuevo(char* nombre, int bloque_inicial){
    t_diccionario* FCB;

    char* path = crear_ruta(nombre);
    int fd = open(path, O_CREAT | O_RDWR, 0664);

    ftruncate(fd,sizeof(t_metadata));

    void* map = mmap(NULL, sizeof(t_metadata), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    t_metadata* metadata = (t_metadata*)map;
    metadata->bloque_inicial=bloque_inicial;

    FCB = malloc(sizeof(t_diccionario));
    
    FCB->map = map;
    FCB->size = sizeof(t_metadata);
    FCB->fd = fd;
    FCB->map->bloque_inicial=bloque_inicial;
    FCB->map->tam_archivo=0;

    dictionary_put(diccionarioFS, nombre, FCB);

    close(fd);
    return;
}


t_diccionario* getFCBxInicio(int bloque_inicio){
    t_list* lista= dictionary_elements(diccionarioFS);
    for(int i=0;i<list_size(lista);i++){
        t_diccionario* FCB = list_get(lista,i);
        if(FCB->map->bloque_inicial==bloque_inicio){
            return FCB;
        }
    }
    return NULL;
}

    

