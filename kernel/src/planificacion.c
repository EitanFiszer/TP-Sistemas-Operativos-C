#include "planificacion.h"

//colas
t_list* lista_new;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* cola_exec;

//semaforos de colas

pthread_mutex_t sem_q_new;
pthread_mutex_t sem_q_ready;
pthread_mutex_t sem_q_ready_priori;
pthread_mutex_t  sem_q_blocked;
pthread_mutex_t  sem_q_exit;
pthread_mutex_t  sem_q_exec;
pthread_mutex_t  sem_CPU_libre;
sem_t sem_cont_ready;
int PID=0;

void *planificacion(void *args){
    iniciar_colas();
    iniciar_semaforos();

    if(strcmp(algoritmo_planificacion, "FIFO") == 0){
        stl_FIFO();
    }else if(strcmp(algoritmo_planificacion, "RR") == 0){
        stl_RR();
    }else if (strcmp(algoritmo_planificacion, "VRR") == 0){
        stl_VRR();
    }else{
        log_error(logger, "Error con el algoritmo de planificacion enviado");
    }

}

void iniciar_colas(void)
{
    lista_new = list_create();
    cola_ready = queue_create();
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
}

void iniciar_proceso(char *path) //PLANIFICADOR A LARGO PLAZO 
{
    // CREO EL PROCESO LOG
    log_info(logger, "Se crea el proceso <%d> en NEW", PID);
    // creo el paquete con las instrucciones para enviar a memoria las instrucciones
    enviar_instrucciones_memoria(path, PID, resultHandshakeMemoria);
    // creo la PCB Y la guardo en cola NEW
    t_PCB *new_PCB = crear_PCB(PID);

    // semaforo cola new
    pthread_mutex_lock(&sem_q_new);
    list_add_in_index(lista_new, PID, new_PCB);
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
    newPCB->quantum = 0;
    newPCB->estado = NEW;
    return newPCB;
}

void cargar_ready_por_pid(int num_pid) //PLANIFICADOR A LARGO PLAZO 
{
    // semaforo cola new
    pthread_mutex_lock(&sem_q_new);
    t_PCB *retirar_PCB = list_remove(lista_new, num_pid);
    pthread_mutex_unlock(&sem_q_new);

    if (retirar_PCB == NULL)
    {
        log_error(logger, "NO EXISTE ELEMENTO EN LA LISTA READY CON PID: %d", num_pid);
    }
    else
    {
        retirar_PCB->estado = READY;

        pthread_mutex_lock(&sem_q_ready);
        queue_push(cola_ready, retirar_PCB);
        pthread_mutex_unlock(&sem_q_ready);

        log_info(logger, "PID:%d - Estado Anterior: NEW - Estado Actual: READY", retirar_PCB->PID);

        sem_post(&sem_cont_ready);
    }
}
void cargar_ready(t_PCB* pcb) 
{
        pcb->estado = READY;

        pthread_mutex_lock(&sem_q_ready);
        queue_push(cola_ready, pcb);
        pthread_mutex_unlock(&sem_q_ready);

        log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: READY", pcb->PID);

        sem_post(&sem_cont_ready);
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
        enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready, resultHandshakeDispatch);

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
        retirar_ready->quantum = quantum;

        pthread_mutex_lock(&sem_q_exec);
        queue_push(cola_exec, retirar_ready);
        pthread_mutex_unlock(&sem_q_exec);
        log_info(logger, "PID:%d - Estado Anterior: READY - Estado Actual: EXEC", retirar_ready->PID);
        // envio proceso a cpu
        enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready, resultHandshakeDispatch);

        usleep(quantum);
        // interrumpo el proceso por fin de quantum
        interrumpir(resultHandshakeInterrupt);
    }
}

void stl_VRR()
{
}

void lts_ex(t_PCB* pcb)
{
    pthread_mutex_lock(&sem_q_exit);
    queue_push(cola_exit, pcb);
    pthread_mutex_unlock(&sem_q_exit);
    //avisar a memoria que elimine instrucciones de PID
    enviar_paquete_memoria(FINALIZAR_PROCESO, pcb->PID,resultHandshakeMemoria);
}

void desalojar(){
    pthread_mutex_lock(&sem_q_exec);
    t_PCB *retirar_PCB = queue_pop(cola_exec);
    pthread_mutex_unlock(&sem_q_exec);
    //CPU LIBRE
    pthread_mutex_unlock(&sem_CPU_libre);
}

void atender_wait(t_PCB* pcb, char* recurso){
    
}
void atender_signal(t_PCB* pcb, char* recurso){
    
}