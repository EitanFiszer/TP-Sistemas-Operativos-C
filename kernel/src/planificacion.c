#include "planificacion.h"
#include <stdbool.h>

// DETENER E INICIAR PLANI
bool planificacion_activa = true;
pthread_mutex_t mutex_planificacion;
pthread_cond_t cond_planificacion;

// colas
t_list *lista_new;
t_queue *cola_ready;
t_queue *cola_ready_priori;

t_queue *cola_blocked;
t_queue *cola_exit;
t_queue *cola_exec;

// semaforos de colas

pthread_mutex_t sem_q_new;
pthread_mutex_t sem_q_ready;
pthread_mutex_t sem_q_ready_priori;
pthread_mutex_t sem_q_blocked;
pthread_mutex_t sem_q_exit;
pthread_mutex_t sem_q_exec;
pthread_mutex_t sem_CPU_libre;
pthread_mutex_t interrupcion_syscall;
bool interrumpio_syscall = false;

sem_t sem_cont_ready;

sem_t sem_gm_actual; // semaforo grado de multiprogramacion

int PID = 0;

t_temporal *tempo_quantum;

// hilo para manejar el quantum

pthread_t hilo_quantum;

// VARIABLE PARA BUSCAR PCBS EN NEW
int global_pid_to_find;

void *planificacion(void *args)
{
    iniciar_colas();
    iniciar_semaforos();

    if (strcmp(algoritmo_planificacion, "FIFO") == 0)
    {
        stl_FIFO();
    }
    else if (strcmp(algoritmo_planificacion, "RR") == 0)
    {
        stl_RR();
    }
    else if (strcmp(algoritmo_planificacion, "VRR") == 0)
    {
        stl_VRR();
    }
    else
    {
        log_error(logger, "Error con el algoritmo de planificación enviado");
    }
    return NULL;
}

void modificar_quantum(t_PCB *pcb)
{
    pthread_cancel(hilo_quantum);
    int64_t tiempo_gastado = temporal_gettime(tempo_quantum);
    pcb->quantum -= tiempo_gastado;
}

void iniciar_colas(void)
{
    lista_new = list_create();
    cola_ready = queue_create();
    cola_ready_priori = queue_create();
    cola_blocked = queue_create();
    cola_exec = queue_create();
    cola_exit = queue_create();
}

void iniciar_semaforos(void)
{
    pthread_mutex_init(&sem_q_new, NULL);
    pthread_mutex_init(&sem_q_ready, NULL);
    pthread_mutex_init(&sem_q_ready_priori, NULL);
    pthread_mutex_init(&sem_q_blocked, NULL);
    pthread_mutex_init(&sem_q_exit, NULL);
    pthread_mutex_init(&sem_q_exec, NULL);

    pthread_mutex_init(&sem_CPU_libre, NULL);
    pthread_mutex_init(&interrupcion_syscall, NULL);

    pthread_mutex_init(&mutex_planificacion, NULL);
    pthread_cond_init(&cond_planificacion, NULL);

    sem_init(&sem_cont_ready, 0, 0);

    sem_init(&sem_gm_actual, 0, grado_multiprog);
}
void eliminar_semaforos()
{
    pthread_mutex_destroy(&sem_q_new);
    pthread_mutex_destroy(&sem_q_ready);
    pthread_mutex_destroy(&sem_q_ready_priori);
    pthread_mutex_destroy(&sem_q_blocked);
    pthread_mutex_destroy(&sem_q_exit);
    pthread_mutex_destroy(&sem_q_exec);

    pthread_mutex_destroy(&sem_CPU_libre);
    pthread_mutex_destroy(&interrupcion_syscall);

    pthread_mutex_destroy(&mutex_planificacion);
    pthread_cond_destroy(&cond_planificacion);
    sem_destroy(&sem_cont_ready);
    sem_destroy(&sem_gm_actual);
}
void eliminar_colas()
{
    queue_clean_and_destroy_elements(cola_blocked, free);
    queue_clean_and_destroy_elements(cola_exec, free);
    queue_clean_and_destroy_elements(cola_ready, free);
    queue_clean_and_destroy_elements(cola_ready_priori, free);
    queue_clean_and_destroy_elements(cola_exit, free);
    list_destroy_and_destroy_elements(lista_new, free);
}

void iniciar_proceso(char *path)
{
    log_info(logger, "Se crea el proceso con el path %s <%d> en NEW", path, PID);
    enviar_instrucciones_memoria(path, PID);
    t_PCB *new_PCB = crear_PCB(PID);

    pthread_mutex_lock(&sem_q_new);
    list_add(lista_new, new_PCB);
    pthread_mutex_unlock(&sem_q_new);

    PID++;
}

t_PCB *crear_PCB(int num_pid)
{
    t_PCB *newPCB = malloc(sizeof(t_PCB));
    newPCB->PID = num_pid;
    newPCB->program_counter = 0;
    memset(&newPCB->cpu_registro, 0, sizeof(newPCB->cpu_registro));
    newPCB->quantum = quantum;
    newPCB->estado = NEW;
    return newPCB;
}

bool condition_pcb_find(void *v_pcb)
{
    t_PCB *pcb = (t_PCB *)v_pcb;
    return pcb->PID == global_pid_to_find;
}

t_PCB *get_and_remove_pcb(int pid)
{
    global_pid_to_find = pid;
    t_PCB *pcb_find = list_find(lista_new, condition_pcb_find);

    if (pcb_find != NULL)
    {
        list_remove_by_condition(lista_new, condition_pcb_find);
    }

    return pcb_find;
}

void cargar_ready_por_pid(int num_pid)
{
    sem_wait(&sem_gm_actual);

    pthread_mutex_lock(&sem_q_new);
    t_PCB *retirar_PCB = get_and_remove_pcb(num_pid);
    pthread_mutex_unlock(&sem_q_new);

    if (retirar_PCB == NULL)
    {
        log_error(logger, "NO EXISTE ELEMENTO EN LA LISTA READY CON PID: %d", num_pid);
    }
    else
    {
        cargar_ready(retirar_PCB, NEW);
    }
}

void cargar_ready(t_PCB *pcb, t_proceso_estado estado_anterior)
{
    pcb->estado = READY;

    pthread_mutex_lock(&sem_q_ready);
    queue_push(cola_ready, pcb);
    pthread_mutex_unlock(&sem_q_ready);

    sem_post(&sem_cont_ready);

    switch (estado_anterior)
    {
    case NEW:
        log_info(logger, "PID:%d - Estado Anterior: NEW - Estado Actual: READY", pcb->PID);
        break;
    case BLOCKED:
        log_info(logger, "PID:%d - Estado Anterior: BLOCKED - Estado Actual: READY", pcb->PID);
        break;
    case EXEC:
        log_info(logger, "PID:%d - Estado Anterior: EXECUTE - Estado Actual: READY", pcb->PID);
        break;
    default:
        break;
    }
}

void stl_FIFO()
{
    while (1)
    {
        pthread_mutex_lock(&mutex_planificacion);
        while (!planificacion_activa)
        {
            log_info(logger, "Planificación detenida, esperando para reanudar");
            pthread_cond_wait(&cond_planificacion, &mutex_planificacion);
        }
        pthread_mutex_unlock(&mutex_planificacion);

        sem_wait(&sem_cont_ready);
        pthread_mutex_lock(&sem_CPU_libre);

        pthread_mutex_lock(&sem_q_ready);
        t_PCB *retirar_ready = queue_pop(cola_ready);
        pthread_mutex_unlock(&sem_q_ready);

        retirar_ready->estado = EXEC;

        pthread_mutex_lock(&sem_q_exec);
        queue_push(cola_exec, retirar_ready);
        pthread_mutex_unlock(&sem_q_exec);
        log_info(logger, "PID:%d - Estado Anterior: READY - Estado Actual: EXEC", retirar_ready->PID);

        enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready, sizeof(t_PCB));
    }
}

void stl_RR()
{
    log_info(logger, "INICIANDO PLANIFICACION POR RR");
    while (1)
    {
        pthread_mutex_lock(&mutex_planificacion);
        while (!planificacion_activa)
        {
            log_info(logger, "Planificación detenida, esperando para reanudar");
            pthread_cond_wait(&cond_planificacion, &mutex_planificacion);
        }
        pthread_mutex_unlock(&mutex_planificacion);
        sem_wait(&sem_cont_ready);
        log_info(logger, "HAY ELEMENTOS EN LA COLA READY");

        pthread_mutex_lock(&sem_CPU_libre);
        log_info(logger, "Bloqueando CPU");

        pthread_mutex_lock(&sem_q_ready);
        t_PCB *retirar_ready = queue_pop(cola_ready);
        pthread_mutex_unlock(&sem_q_ready);

        retirar_ready->estado = EXEC;

        pthread_mutex_lock(&sem_q_exec);
        queue_push(cola_exec, retirar_ready);
        pthread_mutex_unlock(&sem_q_exec);
        log_info(logger, "PID:%d - Estado Anterior: READY - Estado Actual: EXEC", retirar_ready->PID);
        log_info(logger, "QUANTUM PROCESO A EJECUTAR: %d", retirar_ready->quantum);

        if (retirar_ready->quantum <= 0)
        {
            retirar_ready->estado = READY;
            retirar_ready->quantum = quantum;

            pthread_mutex_lock(&sem_q_ready);
            queue_push(cola_ready, retirar_ready);
            pthread_mutex_unlock(&sem_q_ready);
            log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: READY, FIN DE QUANTUM", retirar_ready->PID);
        }
        else
        {
            tempo_quantum = temporal_create();

            pthread_create(&hilo_quantum, NULL, manejar_quantum, (void *)retirar_ready);

            enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready, sizeof(t_PCB));

            pthread_join(hilo_quantum, NULL);
        }
    }
}

void *manejar_quantum(void *arg)
{
    t_PCB *pcb = (t_PCB *)arg;

    sleep(((unsigned int)(pcb->quantum)) / 1000);
    interrumpir(FIN_QUANTUM);
    temporal_destroy(tempo_quantum);
    return NULL;
}

void hubo_syscall(t_PCB *pcb)
{
    pthread_cancel(hilo_quantum);
    int64_t tiempo_gastado = temporal_gettime(tempo_quantum);
    pcb->quantum -= tiempo_gastado;

    if (strcmp(algoritmo_planificacion, "VRR") == 0)
    {
        pthread_mutex_lock(&sem_q_ready_priori);
        queue_push(cola_ready_priori, pcb);
        pthread_mutex_unlock(&sem_q_ready_priori);
    }
    else
    {
        pthread_mutex_lock(&sem_q_ready);
        queue_push(cola_ready, pcb);
        pthread_mutex_unlock(&sem_q_ready);
    }

    sem_post(&sem_cont_ready);
    temporal_destroy(tempo_quantum);
}

void stl_VRR()
{
    while (1)
    {
        pthread_mutex_lock(&mutex_planificacion);
        while (!planificacion_activa)
        {
            log_info(logger, "Planificación detenida, esperando para reanudar");
            pthread_cond_wait(&cond_planificacion, &mutex_planificacion);
        }
        pthread_mutex_unlock(&mutex_planificacion);
        sem_wait(&sem_cont_ready);

        pthread_mutex_lock(&sem_CPU_libre);

        pthread_mutex_lock(&sem_q_ready_priori);
        int largo_priori = queue_size(cola_ready_priori);
        pthread_mutex_unlock(&sem_q_ready_priori);
        t_PCB *retirar_ready;
        if (largo_priori > 0)
        {
            pthread_mutex_lock(&sem_q_ready_priori);
            retirar_ready = queue_pop(cola_ready_priori);
            pthread_mutex_unlock(&sem_q_ready_priori);
        }
        else
        {
            pthread_mutex_lock(&sem_q_ready);
            retirar_ready = queue_pop(cola_ready);
            pthread_mutex_unlock(&sem_q_ready);
        }

        retirar_ready->estado = EXEC;

        pthread_mutex_lock(&sem_q_exec);
        queue_push(cola_exec, retirar_ready);
        pthread_mutex_unlock(&sem_q_exec);
        log_info(logger, "PID:%d - Estado Anterior: READY - Estado Actual: EXEC", retirar_ready->PID);

        if (quantum < 0)
        {
            // replanifico FIN DE QUANTUM
            retirar_ready->estado = READY;
            retirar_ready->quantum = quantum;

            pthread_mutex_lock(&sem_q_ready);
            queue_push(cola_ready, retirar_ready);
            pthread_mutex_unlock(&sem_q_ready);
            log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: READY, FIN DE QUANTUM", retirar_ready->PID);
        }
        else
        {
            // empiezo a cronometrar el tiempo
            tempo_quantum = temporal_create();

            // creo hilo para manejar el quantum
            pthread_create(&hilo_quantum, NULL, manejar_quantum, (void *)retirar_ready);

            // envio proceso a cpu
            enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready, sizeof(t_PCB));

            pthread_join(hilo_quantum, NULL);
        }
    }
}

void enviar_new_exit(int pid)
{
    pthread_mutex_lock(&sem_q_new);
    t_PCB *retirar_PCB = get_and_remove_pcb(pid);
    pthread_mutex_unlock(&sem_q_new);
    lts_ex(retirar_PCB, NEW, "OUT_OF_MEMORY");
}

void lts_ex(t_PCB *pcb, t_proceso_estado estado_anterior, char *motivo)
{
    if (estado_anterior != NEW)
    {
        sem_post(&sem_gm_actual);
    }
    modificar_fin_proc_dic_rec(pcb->PID);

    pcb->estado = EXIT;
    pthread_mutex_lock(&sem_q_exit);
    queue_push(cola_exit, pcb);
    pthread_mutex_unlock(&sem_q_exit);

    enviar_paquete_memoria(FINALIZAR_PROCESO, &pcb->PID, sizeof(int));

    log_info(logger,"Finaliza el proceso <%d> - Motivo: <%s>", pcb->PID, motivo);

    switch (estado_anterior)
    {
    case NEW:
        log_info(logger, "PID:%d - Estado Anterior: NEW - Estado Actual: EXIT", pcb->PID);
        break;
    case BLOCKED:
        log_info(logger, "PID:%d - Estado Anterior: BLOCKED - Estado Actual: EXIT", pcb->PID);
        break;
    case EXEC:
        log_info(logger, "PID:%d - Estado Anterior: EXECUTE - Estado Actual: EXIT", pcb->PID);
        break;
    case READY:
        log_info(logger, "PID:%d - Estado Anterior: READY - Estado Actual: EXIT", pcb->PID);
        break;
    default:
        break;
    }
}

void desalojar()
{
    // FALLA EN MEMORIA_3
    pthread_mutex_lock(&sem_q_exec);
    t_PCB *retirar_PCB = queue_pop(cola_exec);
    pthread_mutex_unlock(&sem_q_exec);

    log_info(logger, "DESALOJANDO PROCESO DEJANDO CPU_LIBRE");
    pthread_mutex_unlock(&sem_CPU_libre);
    log_info(logger, "CPU_LIBRE, PID SACADO: %d", retirar_PCB->PID);
    free(retirar_PCB);
}

bool buscar_pcb(int pid)
{

    t_PCB *pcb = malloc(sizeof(t_PCB));
    bool encontrado = false;
    // RECORRO NEW
    pthread_mutex_lock(&sem_q_new);
    if (!list_is_empty(lista_new))
    {
        t_list_iterator *iterador = list_iterator_create(lista_new);

        while (list_iterator_has_next(iterador) && !encontrado)
        {
            pcb = list_iterator_next(iterador);
            if (pcb->PID == pid)
            {
                int pos = list_iterator_index(iterador);
                pcb = list_remove(lista_new, pos);
                lts_ex(pcb, NEW, "INTERRUPTED_BY_USER");
                encontrado = true;
            }
        }
        list_iterator_remove(iterador);
    }
    pthread_mutex_unlock(&sem_q_new);

    // RECORRO READY
    pthread_mutex_lock(&sem_q_ready);
    if (!queue_is_empty(cola_ready))
    {
        int tam_cola = queue_size(cola_ready);
        for (int i = 0; i < tam_cola && !encontrado; i++)
        {
            pcb = queue_pop(cola_ready);
            if (pcb->PID == pid)
            {
                lts_ex(pcb, READY, "INTERRUPTED_BY_USER");
                encontrado = true;
            }
            else
            {
                queue_push(cola_ready, pcb);
            }
        }
    }
    pthread_mutex_unlock(&sem_q_ready);

    // RECORRO READY PRIORI
    if (strcmp(algoritmo_planificacion, "VRR") == 0)
    {
        pthread_mutex_lock(&sem_q_ready_priori);
        if (!queue_is_empty(cola_ready_priori))
        {
            int tam_cola = queue_size(cola_ready_priori);
            for (int i = 0; i < tam_cola; i++)
            {
                if (pcb->PID == pid)
                {
                    lts_ex(pcb, READY,"INTERRUPTED_BY_USER");
                    encontrado = true;
                }
                else
                {
                    queue_push(cola_ready_priori, pcb);
                }
            }
        }
        pthread_mutex_unlock(&sem_q_ready_priori);
    }

    // RECORRER BLOCKED
    pthread_mutex_lock(&sem_q_blocked);
    if (!queue_is_empty(cola_blocked))
    {
        int tam_cola = queue_size(cola_blocked);
        str_blocked *dat_bloc = malloc(sizeof(dat_bloc));
        for (int i = 0; i < tam_cola && !encontrado; i++)
        {
            dat_bloc = queue_pop(cola_blocked);
            if (dat_bloc->pcb->PID == pid)
            {
                sacar_bloqueo(dat_bloc);
                lts_ex(pcb, BLOCKED,"INTERRUPTED_BY_USER");
                encontrado = true;
            }
            else
            {
                queue_push(cola_blocked, dat_bloc);
            }
        }
    }
    pthread_mutex_unlock(&sem_q_blocked);
    return encontrado;
}

void sacar_bloqueo(str_blocked *dat_bloc)
{
    switch (dat_bloc->tipo)
    {
    case IOB:
        remove_cola_blocked_io(dat_bloc->key, dat_bloc->pcb);
        break;
    case REC:
        remove_cola_blocked_rec(dat_bloc->key, dat_bloc->pcb);
        break;
    }
}

void finalizar_proceso(int pid)
{
    bool encontrado = buscar_pcb(pid);
    if (encontrado)
    {
        printf("Finaliza el proceso <%d> - Motivo: INTERRUPTED_BY_USER", pid);
    }
}
void detener_planificacion()
{
    pthread_mutex_lock(&mutex_planificacion);
    planificacion_activa = false;
    pthread_mutex_unlock(&mutex_planificacion);
    printf("Planificación detenida");
}

void iniciar_planificacion()
{
    pthread_mutex_lock(&mutex_planificacion);
    planificacion_activa = true;
    pthread_cond_broadcast(&cond_planificacion);
    pthread_mutex_unlock(&mutex_planificacion);
    printf("Planificación iniciada");
}

void modificar_multiprogramacion(int num)
{
    if (num > 0 && num < 30)
    {
        if (num < grado_multiprog)
        {
            for (int i = 0; i < grado_multiprog - num; i++)
            {
                sem_wait(&sem_gm_actual);
            }
        }
        else if (num > grado_multiprog)
        {
            for (int i = 0; i < num - grado_multiprog; i++)
            {
                sem_post(&sem_gm_actual);
            }
        }
    }
    else
    {
        printf("Grado de multiprogramacion incorrecto");
    }
}

void listar_procesos_por_estado()
{
    t_PCB *pcb = malloc(sizeof(t_PCB));
    // MUESTRO NEW
    pthread_mutex_lock(&sem_q_new);
    if (list_is_empty(lista_new))
    {
        printf("No hay procesos en estado: NEW \n");
    }
    else
    {
        printf("Procesos en estado: NEW \n");
        t_list_iterator *iterador = list_iterator_create(lista_new);

        while (list_iterator_has_next(iterador))
        {
            pcb = list_iterator_next(iterador);
            printf("PID: <%d> \n", pcb->PID);
        }
        list_iterator_remove(iterador);
    }
    pthread_mutex_unlock(&sem_q_new);
    // MUESTRO READY
    pthread_mutex_lock(&sem_q_ready);
    if (!queue_is_empty(cola_ready))
    {
        printf("Procesos en estado: READY \n");
        int tam_cola = queue_size(cola_ready);
        for (int i = 0; i < tam_cola; i++)
        {
            pcb = queue_pop(cola_ready);
            printf("PID: <%d> \n", pcb->PID);
            queue_push(cola_ready, pcb);
        }
    }
    else
    {
        printf("No hay procesos en estado: READY \n");
    }
    pthread_mutex_unlock(&sem_q_ready);
    if (strcmp(algoritmo_planificacion, "VRR") == 0)
    {
        pthread_mutex_lock(&sem_q_ready_priori);
        if (!queue_is_empty(cola_ready_priori))
        {
            printf("Procesos en estado: READY PRIORIDAD \n");
            int tam_cola = queue_size(cola_ready_priori);
            for (int i = 0; i < tam_cola; i++)
            {
                pcb = queue_pop(cola_ready_priori);
                printf("PID: <%d> \n", pcb->PID);
                queue_push(cola_ready_priori, pcb);
            }
        }
        pthread_mutex_unlock(&sem_q_ready_priori);
    }

    // MUESTRO BLOCKED
    pthread_mutex_lock(&sem_q_blocked);
    if (!queue_is_empty(cola_blocked))
    {
        printf("Procesos en estado: BLOCKED \n");
        int tam_cola = queue_size(cola_blocked);
        str_blocked *dat_bloc = malloc(sizeof(str_blocked));
        for (int i = 0; i < tam_cola; i++)
        {
            // TODO: NO ANDA
            dat_bloc = queue_pop(cola_blocked);
            printf("PID: <%d> \n", dat_bloc->pcb->PID);
            queue_push(cola_blocked, dat_bloc);
        }
    }
    else
    {
        printf("No hay procesos en estado: BLOCKED \n");
    }
    pthread_mutex_unlock(&sem_q_blocked);
    // MUESTRO EXEC
    pthread_mutex_lock(&sem_q_exec);
    if (!queue_is_empty(cola_exec))
    {
        printf("Procesos en estado: EXECUTE \n");
        int tam_cola = queue_size(cola_exec);
        for (int i = 0; i < tam_cola; i++)
        {
            pcb = queue_pop(cola_exec);
            printf("PID: <%d> \n", pcb->PID);
            queue_push(cola_exec, pcb);
        }
    }
    else
    {
        printf("No hay procesos en estado: EXECUTE \n");
    }
    pthread_mutex_unlock(&sem_q_exec);
    // MUESTRO EXIT
    pthread_mutex_lock(&sem_q_exit);
    if (!queue_is_empty(cola_exit))
    {
        printf("Procesos en estado: EXIT \n");
        int tam_cola = queue_size(cola_exit);
        for (int i = 0; i < tam_cola; i++)
        {
            pcb = queue_pop(cola_exit);
            printf("PID: <%d> \n", pcb->PID);
            queue_push(cola_exit, pcb);
        }
    }
    else
    {
        printf("No hay procesos en estado: EXIT \n");
    }
    pthread_mutex_unlock(&sem_q_exit);
}

void add_queue_blocked(t_PCB *pcb, tipo_block tipo, char *key)
{
    str_blocked dat_bloc;
    dat_bloc.pcb = pcb;
    dat_bloc.tipo = tipo;
    dat_bloc.key = key;
    pthread_mutex_lock(&sem_q_blocked);
    queue_push(cola_blocked, &dat_bloc);
    pthread_mutex_unlock(&sem_q_blocked);
}

void delete_queue_blocked(t_PCB *pcb)
{
    str_blocked *dat_bloc = malloc(sizeof(str_blocked));

    pthread_mutex_lock(&sem_q_blocked);

    int tam_cola = queue_size(cola_blocked);

    for (int i = 0; i < tam_cola; i++)
    {
        dat_bloc = queue_pop(cola_blocked);
        if (dat_bloc->pcb->PID != pcb->PID)
        {
            queue_push(cola_blocked, dat_bloc);
        }
        else
        {
            free(dat_bloc);
        }
    }
    pthread_mutex_unlock(&sem_q_blocked);
}