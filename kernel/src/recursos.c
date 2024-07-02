#include "recursos.h"
#include <stdlib.h>
#include "global.h"

// Inicializa el diccionario en el punto de entrada principal de tu aplicación



void guardar_dictionary_recursos(t_config* config){  
    //creo el diccionario para guardar recursos
    diccionario_recursos = dictionary_create();

    char** nombres_r = config_get_array_value(config,"RECURSOS");
    char** instancias_r = config_get_array_value(config,"INSTANCIAS_RECURSOS");

//agrego a diccionario 
    for(int i =0; nombres_r[i]!=NULL && instancias_r[i]!= NULL; i++){
        t_recurso* recurso_agregar = malloc(sizeof(t_recurso));
        recurso_agregar->nombre_recurso = nombres_r[i];
        recurso_agregar->instancias_recurso = atoi(instancias_r[i]);
        recurso_agregar->mutex_recurso = malloc(sizeof(pthread_mutex_t));
        recurso_agregar->cola_blocked_recurso=queue_create();

        pthread_mutex_init(recurso_agregar->mutex_recurso,NULL);

        dictionary_put(diccionario_recursos,nombres_r[i],recurso_agregar);
    }

}

void atender_wait(t_PCB *pcb, char *nombre_recurso)
{
    t_recurso* recurso_encontrado = dictionary_get(diccionario_recursos, nombre_recurso);

    if(recurso_encontrado==NULL){
        //NO EXISTE RECURSO
        log_info(logger , "Recurso %s no encontrado, se desaloja el proceso con PID: %d", nombre_recurso,pcb->PID);
        desalojar();
        // log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb->PID);
        lts_ex(pcb,EXEC);
    }else{
        pthread_mutex_lock(recurso_encontrado->mutex_recurso);

        recurso_encontrado->instancias_recurso --;
        
        if(recurso_encontrado->instancias_recurso<0){
            //desalojo el proceso y lo envio a la cola de bloqueados
            desalojar();
            queue_push(recurso_encontrado->cola_blocked_recurso, pcb);
            log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb->PID);
                            //DEPENDE EL ALGORITMO VER TEMA QUANTUM
        }else{
            //RECURSO ASIGNADO SEGUIR CON EJECUCION
        }

        pthread_mutex_unlock(recurso_encontrado->mutex_recurso);
    }
    
}



void atender_signal(t_PCB *pcb, char *nombre_recurso)
{
    t_recurso* recurso_encontrado = dictionary_get(diccionario_recursos, nombre_recurso);

    if(recurso_encontrado==NULL){
        //NO EXISTE RECURSO
        log_info(logger , "Recurso %s no encontrado, se desaloja el proceso con PID: %d", nombre_recurso,pcb->PID);
        desalojar();
        lts_ex(pcb,EXEC);
    }else{

        pthread_mutex_lock(recurso_encontrado->mutex_recurso);

        recurso_encontrado->instancias_recurso ++;
        
        // PUEDO DESBLOQUEAR UN PROCESO SI HAY INSTANCIAS DE RECURSOS
        if(recurso_encontrado->instancias_recurso>= 0){
            //SI HAY RECURSOS EN LA COLA DE BLOQUEADOS
            if(queue_size(recurso_encontrado->cola_blocked_recurso)>0){
                t_PCB* retirar_bloqueo = queue_pop(recurso_encontrado->cola_blocked_recurso);
                //DEPENDE EL ALGORITMO VER TEMA QUANTUM
                cargar_ready(retirar_bloqueo, BLOCKED);
               
            }
        }

        //SEGUIR CON LA EJECUCION TODO OK

        pthread_mutex_unlock(recurso_encontrado->mutex_recurso);
    }
    
}