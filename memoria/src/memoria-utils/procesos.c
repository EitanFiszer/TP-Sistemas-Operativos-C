#include "procesos.h"
#include <commons/log.h>
#include <stdlib.h>

struct proceso_t {
    int id;
};


proceso_t* crear_proceso(int id, t_log* logger) {
    proceso_t* proceso = (proceso_t*)malloc(sizeof(proceso_t));
    proceso->id = id;
    log_info(logger, "Se creó un proceso con ID: %d", id);
    return proceso;
}

char* leer_instrucciones(const char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "r");
    
    fseek(archivo, 0, SEEK_END);
    long tam_archivo = ftell(archivo);
    fseek(archivo, 0, SEEK_SET);

    char* instrucciones = (char*)malloc(tam_archivo + 1); // +1 para el carácter nulo terminador
    fread(instrucciones, 1, tam_archivo, archivo);
    instrucciones[tam_archivo] = '\0'; // Agregar el carácter nulo terminador
    
    fclose(archivo);
    return instrucciones;
}



// bool finalizar_proceso(int id, t_log* logger) {
//     if (memoria.procesos[id] == NULL) {
//         log_info(logger, "Se intentó finalizar un proceso de ID inexistente: %d¨, id);
//         return false;
//     }
    

//     log_info(logger, ¨Proceso ", id, ¨ finalizado.¨);
//     return true;
// }