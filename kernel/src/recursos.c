#include "recursos.h"
#include <stdlib.h>
#include "global.h"

t_dictionary *rec_por_pid_dic;

void guardar_dictionary_recursos(t_config *config)
{
    rec_por_pid_dic = dictionary_create();
    // creo el diccionario para guardar recursos
    diccionario_recursos = dictionary_create();

    char **nombres_r = config_get_array_value(config, "RECURSOS");
    char **instancias_r = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    // agrego a diccionario
    for (int i = 0; nombres_r[i] != NULL && instancias_r[i] != NULL; i++)
    {
        t_recurso *recurso_agregar = malloc(sizeof(t_recurso));
        recurso_agregar->nombre_recurso = nombres_r[i];
        recurso_agregar->instancias_recurso = atoi(instancias_r[i]);
        recurso_agregar->mutex_recurso = malloc(sizeof(pthread_mutex_t));
        recurso_agregar->cola_blocked_recurso = queue_create();

        pthread_mutex_init(recurso_agregar->mutex_recurso, NULL);

        dictionary_put(diccionario_recursos, nombres_r[i], recurso_agregar);
    }
}

void atender_wait(t_PCB *pcb, char *nombre_recurso)
{
    t_recurso *recurso_encontrado = dictionary_get(diccionario_recursos, nombre_recurso);

    if (recurso_encontrado == NULL)
    {
        // NO EXISTE RECURSO
        log_info(logger, "Recurso %s no encontrado, se desaloja el proceso con PID: %d", nombre_recurso, pcb->PID);
        desalojar();
        // log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb->PID);
        lts_ex(pcb, EXEC);
    }
    else
    {
        pthread_mutex_lock(recurso_encontrado->mutex_recurso);

        recurso_encontrado->instancias_recurso--;

        if (recurso_encontrado->instancias_recurso < 0)
        {
            // desalojo el proceso y lo envio a la cola de bloqueados
            desalojar();
            if (strcmp(algoritmo_planificacion, "RR") == 0 || strcmp(algoritmo_planificacion, "VRR") == 0)
            {
                // / tomo el tiempo en el q hubo syscall
                // cancelo elhilo quantum y cambio el quantum restante parando el cronometro y restando el tiempo del quantum que habia
                modificar_quantum(pcb);
                queue_push(recurso_encontrado->cola_blocked_recurso, pcb);
                add_queue_blocked(pcb, REC, nombre_recurso);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb->PID);
            }
            else
            {
                queue_push(recurso_encontrado->cola_blocked_recurso, pcb);
                add_queue_blocked(pcb, REC, nombre_recurso);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", pcb->PID);
            }
        }
        else
        {
            modificar_wait_dic_rec(pcb->PID, nombre_recurso);
            enviar_paquete_cpu_dispatch(EXEC_PROCESO, pcb, sizeof(t_PCB));
        }

        pthread_mutex_unlock(recurso_encontrado->mutex_recurso);
    }
}

void atender_signal(t_PCB *pcb, char *nombre_recurso)
{
    t_recurso *recurso_encontrado = dictionary_get(diccionario_recursos, nombre_recurso);

    if (recurso_encontrado == NULL)
    {
        // NO EXISTE RECURSO
        log_info(logger, "Recurso %s no encontrado, se desaloja el proceso con PID: %d", nombre_recurso, pcb->PID);
        desalojar();
        lts_ex(pcb, EXEC);
    }
    else
    {

        pthread_mutex_lock(recurso_encontrado->mutex_recurso);

        recurso_encontrado->instancias_recurso++;
        modificar_signal_dic_rec(pcb->PID, nombre_recurso);

        // PUEDO DESBLOQUEAR UN PROCESO SI HAY INSTANCIAS DE RECURSOS
        if (recurso_encontrado->instancias_recurso >= 0)
        {
            // SI HAY RECURSOS EN LA COLA DE BLOQUEADOS
            if (!queue_is_empty(recurso_encontrado->cola_blocked_recurso))
            {
                t_PCB *retirar_bloqueo = queue_pop(recurso_encontrado->cola_blocked_recurso);
                delete_queue_blocked(retirar_bloqueo);
                if (strcmp(algoritmo_planificacion, "VRR") == 0)
                {
                    pthread_mutex_trylock(&sem_q_ready_priori);
                    queue_push(cola_ready_priori, retirar_bloqueo);
                    pthread_mutex_unlock(&sem_q_ready_priori);
                    sem_post(&sem_cont_ready);
                    log_info(logger, "PID:%d - Estado Anterior: BLOCKED - Estado Actual: READY", retirar_bloqueo->PID);
                }
                else
                {
                    cargar_ready(retirar_bloqueo, BLOCKED);
                }
            }
        }

        enviar_paquete_cpu_dispatch(EXEC_PROCESO, pcb, sizeof(t_PCB));

        pthread_mutex_unlock(recurso_encontrado->mutex_recurso);
    }
}

void signal_por_fin(char *nombre_recurso)
{
    t_recurso *recurso_encontrado = dictionary_get(diccionario_recursos, nombre_recurso);

    if (recurso_encontrado != NULL)
    {
        pthread_mutex_lock(recurso_encontrado->mutex_recurso);
        recurso_encontrado->instancias_recurso++;
        // PUEDO DESBLOQUEAR UN PROCESO SI HAY INSTANCIAS DE RECURSOS
        if (recurso_encontrado->instancias_recurso >= 0)
        {
            // SI HAY RECURSOS EN LA COLA DE BLOQUEADOS
            if (!queue_is_empty(recurso_encontrado->cola_blocked_recurso))
            {
                t_PCB *retirar_bloqueo = queue_pop(recurso_encontrado->cola_blocked_recurso);
                delete_queue_blocked(retirar_bloqueo);
                if (strcmp(algoritmo_planificacion, "VRR") == 0)
                {
                    pthread_mutex_trylock(&sem_q_ready_priori);
                    queue_push(cola_ready_priori, retirar_bloqueo);
                    pthread_mutex_unlock(&sem_q_ready_priori);
                    sem_post(&sem_cont_ready);
                    log_info(logger, "PID:%d - Estado Anterior: BLOCKED - Estado Actual: READY", retirar_bloqueo->PID);
                }
                else
                {
                    cargar_ready(retirar_bloqueo, BLOCKED);
                }
            }
        }
        pthread_mutex_unlock(recurso_encontrado->mutex_recurso);
    }
}

int buscar_recurso(t_list *lista, char *nombre)
{
    t_list_iterator *iterador = list_iterator_create(lista);
    t_rec_list *dat_rec = malloc(sizeof(t_rec_list));
    int index = -1;
    while (list_iterator_has_next(iterador))
    {
        dat_rec = list_iterator_next(iterador);
        if (strcmp(dat_rec->nombre_recurso, nombre) == 0)
        {
            index = list_iterator_index(iterador);
            list_iterator_remove(iterador);
            return index;
        }
    }
    return index;
}

void recorrer_liberar_rec(t_list *recurso_list)
{
    t_list_iterator *iterador = list_iterator_create(recurso_list);
    t_rec_list *dat_rec = malloc(sizeof(t_rec_list));
    while (list_iterator_has_next(iterador))
    {
        dat_rec = list_iterator_next(iterador);
        for (int i = 0; i < dat_rec->instancias_recurso; i++)
        {
            signal_por_fin(dat_rec->nombre_recurso);
        }
    }
    list_iterator_remove(iterador);
}

void modificar_wait_dic_rec(int pid, char *nombre_rec)
{
    t_list *recurso_list = dictionary_get(rec_por_pid_dic, int_to_string(pid));
    if (recurso_list == NULL)
    {
        recurso_list = list_create();
        t_rec_list *dat_rec = malloc(sizeof(t_rec_list));
        dat_rec->instancias_recurso = 1;
        dat_rec->nombre_recurso = nombre_rec;
        list_add(recurso_list, dat_rec);
        dictionary_put(rec_por_pid_dic, int_to_string(pid), dat_rec);
    }
    else
    {
        int index_recurso = buscar_recurso(recurso_list, nombre_rec);
        if (index_recurso == -1)
        {
            t_rec_list *dat_rec = malloc(sizeof(t_rec_list));
            dat_rec->instancias_recurso = 1;
            dat_rec->nombre_recurso = nombre_rec;
            list_add(recurso_list, dat_rec);
        }
        else
        {
            t_rec_list *dat_rec = list_get(recurso_list, index_recurso);
            dat_rec->instancias_recurso++;
        }
    }
}

void modificar_signal_dic_rec(int pid, char *nombre_rec)
{
    t_list *recurso_list = dictionary_get(rec_por_pid_dic, int_to_string(pid));
    if (recurso_list != NULL)
    {
        int index_recurso = buscar_recurso(recurso_list, nombre_rec);
        if (index_recurso != -1)
        {
            t_rec_list *dat_rec = list_get(recurso_list, index_recurso);
            dat_rec->instancias_recurso--;
        }
    }
}

// FUNCION PARA DEVOLVER RECURSOS
void modificar_fin_proc_dic_rec(int pid)
{
    t_list *recurso_list = dictionary_get(rec_por_pid_dic, int_to_string(pid));
    if (recurso_list != NULL)
    {
        recorrer_liberar_rec(recurso_list);
        list_destroy_and_destroy_elements(recurso_list, free);
    }
}

void remove_cola_blocked_rec(char *nombre_recurso, t_PCB *pcb)
{
    t_recurso *recurso_encontrado = dictionary_get(diccionario_recursos, nombre_recurso);
    if (recurso_encontrado != NULL)
    {
        pthread_mutex_lock(recurso_encontrado->mutex_recurso);
        t_PCB *pcb_aux = malloc(sizeof(t_PCB));
        int tam_cola_bloc = sizeof(recurso_encontrado->cola_blocked_recurso);
        for (int i = 0; i < tam_cola_bloc; i++)
        {
            pcb_aux = queue_pop(recurso_encontrado->cola_blocked_recurso);
            if (pcb_aux->PID != pcb->PID)
            {
                queue_push(recurso_encontrado->cola_blocked_recurso, pcb_aux);
            }
        }
        pthread_mutex_unlock(recurso_encontrado->mutex_recurso);
    }
}
