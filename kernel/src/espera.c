#include "espera.h"
bool bool_error_memoria=false;
bool bool_syscall=false;
bool bool_interrupted_by_user=false;
bool bool_interrumpi=false;
void atender_cliente(void *socket)
{
    char *nombre_io_hilo = NULL;
    int socket_cliente_IO = *(int *)socket;
    free(socket);
    // bool salir = true;
    while (1)
    {
        if (socket_cliente_IO == -1)
        {
            log_error(logger, "Cliente IO desconectado, socket: %d", socket_cliente_IO);
            if (nombre_io_hilo != NULL)
            {
                desconectar_IO(nombre_io_hilo);
            }
            break;
        }
        t_paquete_entre *unPaquete = recibir_paquete_entre(socket_cliente_IO);
        if (unPaquete == NULL)
        {
            log_error(logger, "Cliente IO desconectado, socket: %d  nombre %s", socket_cliente_IO, nombre_io_hilo);
            if (nombre_io_hilo != NULL)
            {
                desconectar_IO(nombre_io_hilo);
            }
            break;
        }
        else
        {
            switch (unPaquete->operacion)
            {
            case IO_INTERFAZ_CREADA:
                t_payload_interfaz_creada *datos_interfaz = deserializar_interfaz_creada(unPaquete->payload);
                nombre_io_hilo = malloc(strlen(datos_interfaz->nombre) + 1);
                strcpy(nombre_io_hilo, datos_interfaz->nombre);
                agregar_interfaz(datos_interfaz->nombre, datos_interfaz->tipo_interfaz, socket_cliente_IO);
                log_info(logger, "NUEVA INTERFAZ %s CONECTADA", datos_interfaz->nombre);
                break;
            case TERMINE_OPERACION:
                desocupar_io(nombre_io_hilo);
                break;
            default:
                log_error(logger, "no se recibio paquete de la IO, error");
                break;
            }
            // CUANDO SE DESCONECTE
            // free(nombre_io_hilo)
        }
    }
}

void *esperar_paquetes_memoria(void *arg)
{
    // int *socket = (int *)arg;
    // int socketMemoria = *socket;

    while (1)
    {
        t_paquete_entre *unPaquete = recibir_paquete_entre(resultHandshakeMemoria);
        if (unPaquete == NULL)
        {
            log_error(logger, "Hubo un error al recibir paquete de Memoria, cerrando kernel");
            finalizar_kernel();
        }
        else
        {
            int PID_recibido;
            switch (unPaquete->operacion)
            {
            case INSTRUCCIONES_CARGADAS:
                PID_recibido = *(int *)unPaquete->payload;
                log_info(logger, "INSTRUCCIONES CARGADAS del PID: %d", PID_recibido);
                cargar_ready_por_pid(PID_recibido);
                break;
            case ARCHIVO_NO_ENCONTRADO:
                PID_recibido = *(int *)unPaquete->payload;
                log_info(logger, "INSTRUCCIONES NO ENCONTRADAS del PID: %d", PID_recibido);
                enviar_new_exit(PID_recibido);
                // ELIMINAR PROCESO SACAR DE COLA NEW ENVIAR A EXIT -
                break;
            default:
                log_error(logger, "no se recibio paquete de la memoria, error");
                break;
            }
        }
    }
}

void *esperar_paquetes_cpu_dispatch(void *arg)
{
    // int *socket_dispatch = (int *)arg;
    while (1)
    {
        t_paquete_entre *paquete_dispatch = recibir_paquete_entre(resultHandshakeDispatch);
        if(paquete_dispatch==NULL){
            log_error(logger, "Hubo un error al recibir paquete de CPU, cerrando kernel");
            finalizar_kernel();
        }
        switch (paquete_dispatch->operacion)
        {
        case INTERRUMPIO_PROCESO:     // A CHEQUEAR
            log_info(logger, "RECIBIENDO PROCESO DESALOJADO");
            t_PCB *PCB = (t_PCB *)paquete_dispatch->payload;
            if(bool_error_memoria){
                lts_ex(PCB,EXEC,"ERROR_OUT_OF_MEMORY");
                bool_error_memoria = false;
            }else if (bool_syscall){
                log_info(logger, "hubo_syscall");
                bool_syscall = false;
            }else if(bool_interrupted_by_user){
                bool_interrupted_by_user =false;
            }
            else if(bool_interrumpi){
                bool_interrumpi = false;
                desalojar();
                cargar_ready(PCB, EXEC);
            }
            break;
            // pthread_mutex_unlock(&interrupcion_syscall);
        case ERROR_OUT_OF_MEMORY:
            interrumpir(ERROR_OUT_OF_MEMORY_I);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            break;
            
        case WAIT:
            t_payload_wait_signal *paquete_wait = malloc(sizeof(t_payload_wait_signal));
            paquete_wait = deserializar_wait_signal(paquete_dispatch->payload);
            log_info(logger, "RECIBIENDO WAIT DE RECURSO %s, PID: %d", paquete_wait->recurso, paquete_wait->pcb->PID);
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            atender_wait(paquete_wait->pcb, paquete_wait->recurso);
            break;

        case SIGNAL:
            t_payload_wait_signal *paquete_signal = malloc(sizeof(t_payload_wait_signal));
            paquete_signal = deserializar_wait_signal(paquete_dispatch->payload);
            log_info(logger, "RECIBIENDO SIGNAL DE RECURSO %s, PID: %d", paquete_signal->recurso, paquete_signal->pcb->PID);
            atender_signal(paquete_signal->pcb, paquete_signal->recurso);
            break;

        case TERMINO_EJECUCION:
            desalojar();
            t_PCB *pcb_dispatch = (t_PCB *)paquete_dispatch->payload;
            log_info(logger, "Finaliza el proceso %d - Motivo: SUCCESS", pcb_dispatch->PID);
            lts_ex(pcb_dispatch, EXEC,"SUCCESS");
            /// PROCESO TERMINADO SE DESALOJA Y SE ENVIA A EXIT
            break;

        case IO_STDIN_READ:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_io_stdin_read *payload_stdin_read= deserializar_io_stdin_read(paquete_dispatch->payload);
            atender_io_stdin_read(payload_stdin_read);
            break;
        case IO_STDOUT_WRITE:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_io_stdout_write *payload_stdout_write = deserializar_io_stdout_write(paquete_dispatch->payload);
            atender_io_stdout_write(payload_stdout_write);

            break;
        case IO_FS_CREATE:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_fs_create *payload_fs_create = deserializar_fs_create(paquete_dispatch->payload);
            atender_fs_createOrDelate(payload_fs_create, IO_FS_CREATE);
            break;
        case IO_FS_DELETE:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_fs_create *payload_fs_del = deserializar_fs_create(paquete_dispatch->payload);
            atender_fs_createOrDelate(payload_fs_del,IO_FS_DELETE);
            break;
        case IO_FS_TRUNCATE:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_fs_truncate *payload_truncate = deserializar_fs_truncate(paquete_dispatch->payload);
            atender_fs_truncate(payload_truncate);
            break;
        case IO_FS_WRITE:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_fs_writeORread *payload_fs_wOr = deserializar_fs_writeORread(paquete_dispatch->payload);
            atender_fs_writeOrRead(payload_fs_wOr,IO_FS_WRITE);
            break;
        case IO_FS_READ:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_fs_writeORread *payloadRoW = deserializar_fs_writeORread(paquete_dispatch->payload);
            atender_fs_writeOrRead(payloadRoW,IO_FS_READ);
            break;
        case IO_GEN_SLEEP:
            interrumpir(SYSCALL);
            enviar_paquete_cpu_dispatch(CONFIRMAR_SYSCALL,NULL,0);
            desalojar();
            t_payload_io_gen_sleep *payload_gen_sleep = deserializar_io_gen_sleep(paquete_dispatch->payload);
            atender_io_gen_sleep(payload_gen_sleep);
            break;
        default:
            log_error(logger, "no se recibio paquete de la memoria, error");
            break;
        }
    }
    return NULL;
}

void enviar_instrucciones_memoria(char *path, int PID)
{
    printf("enviando instrucciones a memoria, path: %s, PID: %d\n", path, PID);
    t_payload_crear_proceso *payload = malloc(sizeof(t_payload_crear_proceso));
    payload->path = path;
    payload->pid = PID;
    int size_crear;

    void *buffer = serializar_crear_proceso(payload, &size_crear);
    enviar_paquete_entre(resultHandshakeMemoria, CREAR_PROCESO, buffer, size_crear);
}

void enviar_paquete_memoria(OP_CODES_ENTRE operacion, void *payload, int size_payload)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, resultHandshakeMemoria);
    eliminar_paquete(paq);
    free(paquete);
}

void enviar_paquete_cpu_dispatch(OP_CODES_ENTRE operacion, void *payload, int size_payload)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, resultHandshakeDispatch);
    eliminar_paquete(paq);
    free(paquete);
}

void interrumpir(t_motivo_interrupcion motivo)
{
    if (motivo == SYSCALL)
    {
        pthread_mutex_lock(&interrupcion_syscall);
        interrumpio_syscall = true;
        pthread_mutex_unlock(&interrupcion_syscall);
        log_info(logger, "INTERRUMPIENDO PROCESO POR SYSCALL");
        bool_syscall =true;
    }else if(motivo == FIN_QUANTUM){
        log_info(logger, "INTERRUMPIENDO PROCESO POR FIN DE QUANTUM");
        bool_interrumpi=true;
    }else if(motivo == ERROR_OUT_OF_MEMORY_I){
        log_info(logger, "INTERRUMPIENDO PROCESO POR ERROR_OUT_OF_MEMORY");
        bool_error_memoria = true;
    }else if(motivo == INTERRUPTED_BY_USER){
        log_info(logger, "INTERRUMPIENDO PROCESO POR INTERRUPTED_BY_USER");
        bool_interrupted_by_user = true;
    }
    t_paquete *paquete_fin_de_q = crear_paquete();
    t_paquete_entre *fin_q = malloc(sizeof(t_paquete_entre));
    fin_q->operacion = INTERRUMPIR_PROCESO;
    fin_q->size_payload = sizeof(int);
    int instruccion_valida = 1;
    fin_q->payload = &instruccion_valida;
    agregar_paquete_entre_a_paquete(paquete_fin_de_q, fin_q);
    enviar_paquete(paquete_fin_de_q, resultHandshakeInterrupt);
    eliminar_paquete(paquete_fin_de_q);
    free(fin_q);
    log_info(logger, "Se interrumpio el proceso");
}
void finalizar_kernel()
{
    printf("Finalizando KERNEL\n");
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(resultHandshakeDispatch);
    liberar_conexion(resultHandshakeInterrupt);
    liberar_conexion(resultHandshakeMemoria);
    eliminar_semaforos();
    eliminar_colas();

    exit(1);
}