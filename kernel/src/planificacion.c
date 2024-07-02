#include "planificacion.h"

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
        log_error(logger, "Error con el algoritmo de planificacion enviado");
    }
    return NULL;
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

    sem_init(&sem_cont_ready, 0, 0);
    sem_init(&sem_gm_actual, 0, grado_multiprog);
}

void iniciar_proceso(char *path) // PLANIFICADOR A LARGO PLAZO
{
    // CREO EL PROCESO LOG
    log_info(logger, "Se crea el proceso con el path %s <%d> en NEW", path, PID);
    // creo el paquete con las instrucciones para enviar a memoria las instrucciones
    enviar_instrucciones_memoria(path, PID);
    // creo la PCB Y la guardo en cola NEW
    t_PCB *new_PCB = crear_PCB(PID);

    // semaforo cola new
    pthread_mutex_lock(&sem_q_new);
    list_add(lista_new, new_PCB);
    pthread_mutex_unlock(&sem_q_new);

    // Incremento identificador de proceso
    PID++;
}
t_PCB *crear_PCB(int num_pid)
{
    t_PCB *newPCB = malloc(sizeof(t_PCB));
    newPCB->PID = num_pid;
    newPCB->program_counter = 0;
    newPCB->cpu_registro.AX = 0;
    newPCB->cpu_registro.BX = 0;
    newPCB->cpu_registro.CX = 0;
    newPCB->cpu_registro.DX = 0;
    newPCB->cpu_registro.EAX = 0;
    newPCB->cpu_registro.EBX = 0;
    newPCB->cpu_registro.ECX = 0;
    newPCB->cpu_registro.EDX = 0;
    newPCB->cpu_registro.SI = 0;
    newPCB->cpu_registro.DI = 0;
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

void cargar_ready_por_pid(int num_pid) // PLANIFICADOR A LARGO PLAZO
{

    sem_wait(&sem_gm_actual);

    // semaforo cola new
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
        // si hay elemenetos en cola ready && la cpu esta libre
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
        // envio proceso a cpu
        // POR AHORA NO SE SI AGREFAR SEMAFORO ACA
        enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready, sizeof(t_PCB));

        // semaforo de planificacion a corto plazo para replanifica
    }
}

void stl_RR()
{
    while (1)
    {
        // si hay elemenetos en cola ready && la cpu esta libre
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

        if (retirar_ready->quantum < 0)
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

            // // interrumpo el proceso por fin de quantum
            // interrumpir(resultHandshakeInterrupt);
            pthread_join(hilo_quantum, NULL);
        }
    }
}

void *manejar_quantum(void *arg)
{
    t_PCB *pcb = (t_PCB *)arg;

    // pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    // pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    sleep(((unsigned int)(pcb->quantum)) / 1000);
    interrumpir();
    temporal_destroy(tempo_quantum);
    return NULL;
}

void hubo_syscall(t_PCB *pcb)
{
    // tomo el tiempo en el q hubo syscall
    // cancelo elhilo quantum y cambio el quantum restante parando el cronometro y restando el tiempo del quantum que habia
    pthread_cancel(hilo_quantum);
    int64_t tiempo_gastado = temporal_gettime(tempo_quantum);
    pcb->quantum -= pcb->quantum - tiempo_gastado;
    // si es VRR el algoritmo agrego a cola ready priori
    if (strcmp(algoritmo_planificacion, "VRR") == 0)
    {
        pthread_mutex_lock(&sem_q_ready_priori);
        queue_push(cola_ready_priori, pcb);
        pthread_mutex_unlock(&sem_q_ready_priori);
        sem_post(&sem_cont_ready);
    }
    else
    {
        pthread_mutex_lock(&sem_q_ready);
        queue_push(cola_ready, pcb);
        pthread_mutex_unlock(&sem_q_ready);
        sem_post(&sem_cont_ready);
    }
    temporal_destroy(tempo_quantum);
}

void stl_VRR()
{
    while (1)
    {

        // si hay elemenetos en cola ready && la cpu esta libre
        sem_wait(&sem_cont_ready);
        log_info(logger, "Hay procesos en ready");
        pthread_mutex_lock(&sem_CPU_libre);
        log_info(logger, "La CPU está libre");
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

            // // interrumpo el proceso por fin de quantum
            // interrumpir(resultHandshakeInterrupt);
            pthread_join(hilo_quantum, NULL);
        }
    }
}
void enviar_new_exit(int pid)
{
    pthread_mutex_lock(&sem_q_new);
    t_PCB *retirar_PCB = get_and_remove_pcb(pid);
    pthread_mutex_unlock(&sem_q_new);
    lts_ex(retirar_PCB, NEW);
}
void lts_ex(t_PCB *pcb, t_proceso_estado estado_anterior)
{

    sem_post(&sem_gm_actual);
    pcb->estado = EXIT;
    pthread_mutex_lock(&sem_q_exit);
    queue_push(cola_exit, pcb);
    pthread_mutex_unlock(&sem_q_exit);

    // avisar a memoria que elimine instrucciones de PID
    enviar_paquete_memoria(FINALIZAR_PROCESO, &pcb->PID, sizeof(int));

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
    pthread_mutex_lock(&sem_q_exec);
    t_PCB *retirar_PCB = queue_pop(cola_exec);
    pthread_mutex_unlock(&sem_q_exec);
    // CPU LIBRE
    pthread_mutex_unlock(&sem_CPU_libre);
    log_info(logger, "CPU_LIBRE, PID SACADO: %d", retirar_PCB->PID);
    free(retirar_PCB);
}

void atender_syscall(void *args)
{
    t_PCB *pcb = (t_PCB *)args;
    hubo_syscall(pcb);
}
