#include <main.h>
void leer_configs()
{
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    puerto = config_get_int_value(config, "PUERTO_ESCUCHA");
    quantum = config_get_int_value(config, "QUANTUM");
}
void consola_interactiva(void)
{
    log_info(logger, "Iniciando consola");
    int PID = 0;
    while (1)
    {
        printf("Ingrese un comando: \n");
        char *leido = readline(">");

        char **split = string_split(leido, " ");
        int length = string_array_size(split);

        if (string_equals_ignore_case(split[0], "INICIAR_PROCESO"))
        {
            iniciar_proceso(split[1]);
            free(split);
            free(leido);
        }
        else if (string_equals_ignore_case(split[0], "EJECUTAR_SCRIPT"))
        {
        }
        else if (string_equals_ignore_case(split[0], "FINALIZAR_PROCESO"))
        {
        }
        else if (string_equals_ignore_case(split[0], "DETENER_PLANIFICACION"))
        {
        }
        else if (string_equals_ignore_case(split[0], "INICIAR_PLANIFICACION"))
        {
        }
        else if (string_equals_ignore_case(split[0], "MULTIPROGRAMACION"))
        {
        }
        else if (string_equals_ignore_case(split[0], "PROCESO_ESTADO"))
        {
        }
        else
        {
            printf("Escribio un comando incorrecto, los comandos existentes son: \n EJECUTAR_SCRIPT \n INICIAR_PROCESO \n FINALIZAR_PROCESO \n DETENER_PLANIFICACION \n INICIAR_PLANIFICACION \nMULTIPROGRAMACION  \n PROCESO_ESTADO \n ");
        }
    }
}

void iniciar_proceso(char *path)
{
    // CREO EL PROCESO LOG
    log_info(logger, "Se crea el proceso <%d> en NEW", PID);
    // creo el paquete con las instrucciones para enviar a memoria las instrucciones
    enviar_instrucciones_memoria(path);
    // creo la PCB Y la guardo en cola NEW
    t_PCB *new_PCB = crear_PCB(PID);
    // semaforo cola new
    pthread_mutex_lock(&sem_q_new);
    queue_push(cola_new, new_PCB);
    pthread_mutex_unlock(&sem_q_new);
    // Incremento identificador de proceso
    PID++;
}

void LTS_N_R(void)
{
    while (1)
    {
        // si el grado de multiprogramacion lo permite enviar procesos de new a ready y si la memoria ya cargo el proceso
        // SEMAFORO MEMORIA CARGO EL PROCESO
        int grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
        if (grado_multiprogramacion > g_multiprogracion_actual())
        {
            // SEMAFORO COLA NEW

            pthread_mutex_lock(&sem_q_new);
            t_PCB *retirar_new = queue_pop(cola_new);
            pthread_mutex_unlock(&sem_q_new);

            retirar_new->estado = READY;
            pthread_mutex_lock(&sem_q_ready);

            queue_push(cola_ready, retirar_new);
            pthread_mutex_lock(&sem_q_new);

            log_info(logger, "PID:%d - Estado Anterior: NEW - Estado Actual: READY", retirar_new->PID);
            sem_post(&sem_cont_ready);
        }
    }
}
// void LTS_exit (void){
//     while(1){
//         //semaforo que me dice que tengo que eliminar un proceso
//     }
// }
/*void MTS(void){
    //ASIGNAR RECURSOS Y BLOQUEARLOS O DESBLOQUEARLOS
}*/
void STS(void)
{
    while (1)
    {
        wait(&sem_sts_cpu_libre);
        char *algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
        if (strcmp(algoritmo_planificacion, "FIFO") == 0)
        {
            // envio el primer elemento de la cola ready a EXEC
            wait(&sem_cont_ready);

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
            enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready);
            // semaforo de planificacion a corto plazo para replanificar
        }
        if (strcmp(algoritmo_planificacion, "RR") == 0)
        {
            // envio el primer elemento de la cola ready a EXEC
            // SEMAFORO CONTADOR DE ELEMENTOS EN COLA READY, SI NO HAY ELEMENTOS EN READY NO SE EJECUTA ESTE CODIGO SEM WAIT(SEM CONTADOR)

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
            enviar_paquete_cpu_dispatch(EXEC_PROCESO, retirar_ready);

            usleep(quantum);
            // interrumpo el proceso por fin de quantum
            interrumpir();
            // espero recibir el pcb con motivo de desalojo
        }
    }
}

void desalojar(t_PCB *pcb_desalojada)
{
    switch (pcb_desalojada->motivo)
    {
    case INTERRUPT:
        // vuelvo a planificar corto plazo y agrego esta pcb a la cola de ready
        sem_post(&sem_sts_cpu_libre);
        break;
    case SIGNAL:
        /*SI EL MOTIVO ES SIGNAL VERIFICAR QUE EXISTA, SUMARLE UNO, Y SACAR UN PROCESO DE LA COLA DE
         BLOQUEADOS*/
        sem_post(&sem_sts_cpu_libre);
        break;
    case WAIT:
        /* SI EL MOTIVO ES WAIT VERIFICO SI EXISTE RECURSO SOLICITADO Y LE RESTO UNO,
        SI EL NUMERO DE RECURSO ES MENOR A 0
        BLOQUEO EL PROCESO CORRESPONDIENTE AL RECURSO */
        sem_post(&sem_sts_cpu_libre);
        break;
    case FINISH:
        // SI EL MOTIVO ES FINISH ENVIO EL PROCESO A EXIT
        sem_post(&sem_sts_cpu_libre);
        break;
    default:
        break;
    }
}

t_PCB *crear_PCB(int PID)
{
    t_PCB *newPCB = malloc(sizeof(t_PCB));
    newPCB->PID = PID;
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
void iniciar_colas(void)
{
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_blocked = queue_create();
    cola_exec = queue_create();
    cola_exit = queue_create();
}
void iniciar_semaforos(void)
{
    pthread_mutex_init(&sem_q_new, NULL);
    pthread_mutex_init(&sem_q_ready, NULL);
    pthread_mutex_init(&sem_q_blocked, NULL);
    pthread_mutex_init(&sem_q_exit, NULL);
    pthread_mutex_init(&sem_q_exec, NULL);

    sem_init(&sem_sts_cpu_libre, 1, 0);
    sem_init(&sem_lts_proceso_cargado, 0, 0);
    sem_init(&sem_cont_ready, 0, 0);
}
int g_multiprogracion_actual(void)
{
    return queue_size(cola_ready) + queue_size(cola_blocked) + queue_size(cola_exec);
}

// //funciones de conexion
void enviar_instrucciones_memoria(char *path)
{
    t_paquete *nuevo_paquete = crear_paquete();
    t_paquete_entre *instruccion;
    instruccion = malloc(sizeof(t_paquete_entre));
    instruccion->operacion = CREAR_PROCESO;
    payload_crear_proceso *payload = malloc(sizeof(payload_crear_proceso));
    payload->path = path;
    payload->pid = PID;
    instruccion->payload = payload;
    agregar_a_paquete(nuevo_paquete, instruccion, sizeof(t_paquete_entre));
    // envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
    enviar_paquete(nuevo_paquete, resultHandshakeMemoria);

    eliminar_paquete(nuevo_paquete);
}
void esperar_paquetes_cpu_dispatch(void)
{
    while (1)
    {
        t_list *paquete = recibir_paquete(resultHandshakeDispatch);
        t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
        switch (paquete_dispatch->operacion)
        {
        case DESALOJAR:
            desalojar(paquete_dispatch->payload);
            break;

        default:
            log_error(logger, "no se recibio paquete de la CPU, error");
            break;
        }
    }
}
void esperar_paquetes_memoria(void)
{
    while (1)
    {
        t_list *paquete = recibir_paquete(resultHandshakeDispatch);
        t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
        switch (paquete_dispatch->operacion)
        {
        case INSTRUCCIONES_CARGADAS:
            // permite continuar con la planificacion a largo plazo
            break;
        default:
            log_error(logger, "no se recibio paquete de la memoria, error");
            break;
        }
    }
}
void enviar_paquete_cpu_dispatch(OP_CODES_ENTRE operacion, void *payload)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->payload = payload;
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paq, puerto_cpu_dispatch);
    log_info(logger, "PAQUETE CREADO Y ENVIADO A CPU DISPATCH");
    eliminar_paquete(paq);
    free(paquete);
}
void interrumpir(void)
{
    t_paquete *paquete_fin_de_q = crear_paquete;
    t_paquete_entre *fin_q = malloc(sizeof(t_paquete_entre));
    fin_q->operacion = INTERRUMPIR_PROCESO;
    agregar_a_paquete(paquete_fin_de_q, fin_q, sizeof(t_paquete_entre));
    enviar_paquete(paquete_fin_de_q, resultHandshakeInterrupt);
    eliminar_paquete(paquete_fin_de_q);
    free(fin_q);
}

// MAIN
int main(int argc, char *argv[])
{
    // decir_hola("Kernel");
    logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
    config = config_create("kernel.config");

    // leemos las configs

    // LEER TAMBIEN LA MULTIPROGRAMACIÓN
    leer_configs();

    char *stringParaLogger = string_from_format("[KERNEL] Escuchando en el puerto: %d", puerto);
    log_info(logger, stringParaLogger);

    // cliente se conecta al sevidor
    resultHandshakeDispatch = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
    resultHandshakeInterrupt = connectAndHandshake(ip_cpu, puerto_cpu_interrupt, KERNEL, "cpu", logger);

    // ESTE ES EL SOCKET PARA CONECTARSE A LA MEMORIA
    resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);

    // creamos el servidor
    server_fd = iniciar_servidor(puerto_escucha, logger);

    handshake_t res = esperar_cliente(server_fd, logger);
    int modulo = res.modulo;
    int socket_cliente = res.socket;
    switch (modulo)
    {
    case IO:
        log_info(logger, "Se conecto un I/O");
        break;
    default:
        log_error(logger, "Se conecto un cliente desconocido");
        break;
    }

    // INICIO LAS COLAS
    iniciar_colas();
    // INICIO LOS SEMAFOROS
    iniciar_semaforos();
    // creo hilo para que reciba informacion de la consola constantement
    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, consola_interactiva, NULL);
    pthread_detach(hilo_consola);

    // CREO HILO PARA QUE REALICE LA PLANIFICACION DE LARGO PLAZO
    pthread_t hilo_LTS;
    pthread_create(&hilo_LTS, NULL, LTS_N_R, NULL);
    pthread_detach(hilo_LTS);

    // CREO HILO PARA QUE REALICE LA PLANIFICACION DE CORTO PLAZO
    pthread_t hilo_STS;
    pthread_create(&hilo_STS, NULL, STS, NULL);
    pthread_detach(hilo_STS);

    // HILO PARA QUE ESPERA PAQUETES DE LA CPU
    pthread_t hilo_espera_cpu;
    pthread_create(&hilo_espera_cpu, NULL, esperar_paquetes_cpu_dispatch, NULL);
    pthread_detach(hilo_espera_cpu);
    // HILO PARA QUE ESPERE PAQUETES DE LA MEMORIA
    pthread_t hilo_espera_memoria;
    pthread_create(&hilo_espera_memoria, NULL, esperar_paquetes_memoria, NULL);
    pthread_detach(hilo_espera_memoria);
    // SEMAFOROS PARA LAS COLAS
    return 0;
}