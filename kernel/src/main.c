#include <main.h>

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
    
    //INICIO LAS COLAS 
    iniciar_colas();
    // creo hilo para que reciba informacion de la consola constantement
    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, consola_interactiva, NULL);
    pthread_join(hilo_consola,NULL);

    //CREO HILO PARA QUE REALICE LA PLANIFICACION DE LARGO PLAZO 
    pthread_t hilo_LTS;
    pthread_create(&hilo_LTS, NULL, LTS, NULL);
    pthread_detach(hilo_LTS);

    //CREO HILO PARA QUE REALICE LA PLANIFICACION DE CORTO PLAZO 
    pthread_t hilo_STS;
    pthread_create(&hilo_STS, NULL, STS, NULL);
    pthread_detach(hilo_STS);

    //SEMAFOROS PARA LAS COLAS 

    return 0;
}
void leer_configs()
{
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    puerto = config_get_int_value(config, "PUERTO_ESCUCHA");
    quantum = config_get_int_value(config,"QUANTUM");
}

// LEO DE LA CONSOLA
    // SI LEO INCIAR PROCESO LE ENVIO A MEMORIA EL PATH DE INSTRUCCIONES 
    // CREO LA PCB Y GUARDO EL PROCESO EN LA COLA NEW
//GRADO DE MULTIPROGRAMACION
//POR OTRO LADO PLANIFICACION
    //FIFO ENVIO A  EXEC Y CUANDO VUELVEN LOS ENVIO A EXIT
    //RR ENVIO A EXEC (ENVIANDO LA PCB A CPU) SI SE ACABA EL QUANTUM ENVIO POR INTERRUPT LA INTERRUPCION DEL PROCESO
    //REPLANIFICO

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
            //CREO EL PROCESO LOG
            log_info(logger,"Se crea el proceso <%d> en NEW", PID);
            // creo el paquete con las instrucciones para enviar a memoria las instrucciones
            
            t_paquete* nuevo_paquete = crear_paquete();

            t_paquete_entre* instruccion;
            instruccion = malloc(sizeof(t_paquete_entre));
            instruccion->operacion = CREAR_PROCESO;
            payload_crear_proceso* payload = malloc(sizeof(payload_crear_proceso));
            payload->path = split[1];
            payload->pid = PID;
            instruccion->payload = payload;
            agregar_a_paquete(nuevo_paquete, instruccion, sizeof(t_paquete_entre));
            //envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
            enviar_paquete(nuevo_paquete,resultHandshakeMemoria);
            //creo la PCB Y la guardo en cola NEW

//ESPERAR QUE LA MEMORIA ME AVISE QUE CARGO LAS INSTRUCCIONES PARA AGREGAR A NEW LA PCB O PARA PASARLO A LISTO
            t_list* procesoCargado = recibir_paquete(resultHandshakeMemoria);
            if (procesoCargado == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la memoria");
            finalizarCPU(logger, config);
            }        
        
            t_paquete_entre* procesoCargado = list_get(procesoCargado, 0);

            // PREGUNTAR
            // switch (procesoCargado->operacion)
            // {
            // case /* constant-expression */:
            //     /* code */
            //     break;
            
            // default:
            //     break;
            // }

        // Hago el fetch de la instruccion
            t_PCB* new_PCB=crear_PCB(PID);
            queue_push(cola_new,new_PCB);
            //Incremento identificador de proceso
            PID++;
            free(split);
            free(leido);
            //elimino paquete
            eliminar_paquete(nuevo_paquete);
            //libero conexion 
            // liberar_conexion(socket_cliente);
        }
    }
}

void LTS(void){
    while(1){
        //si el grado de multiprogramacion lo permite enviar procesos de new a ready
        int grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
        if (grado_multiprogramacion>g_multiprogracion_actual()){
            t_PCB*  retirar_new = queue_pop(cola_new);
            retirar_new->estado = READY;
            queue_push(cola_ready,retirar_new);
            // cambiar el grado de multiprogramacion
        }
    }
}

void STS(void){
    while(1){
        char* algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
        if(strcmp(algoritmo_planificacion, "FIFO")==0){
            //envio el primer elemento de la cola ready a EXEC
            t_PCB* retirar_ready = queue_pop(cola_ready);
            retirar_ready ->estado = EXEC;
            queue_push(cola_exec, retirar_ready);
            //envio PCB a la CPU
            t_paquete* paquete_pcb = crear_paquete();
            t_paquete_entre* exec = malloc(sizeof(t_paquete_entre));
            //operacion
            exec->operacion=EXEC_PROCESO;
            //envio pcb
            exec->payload = retirar_ready;
            agregar_a_paquete(paquete_pcb, exec, sizeof(t_paquete_entre));
            enviar_paquete(paquete_pcb,resultHandshakeDispatch);
            // espero recibir el pcb con motivo de desalojo 
            //SI EL MOTIVO ES WAIT VERIFICO SI EXISTE RECURSO SOLICITADO Y LE RESTO UNO, 
            // SI EL NUMERO DE RECURSO ES MENOR A 0 
            //BLOQUEO EL PROCESO CORRESPONDIENTE AL RECURSO

            //SI EL MOTIVO ES SIGNAL VERIFICAR QUE EXISTA, SUMARLE UNO, Y SACAR UN PROCESO DE LA COLA DE 
            //BLOQUEADOS
            //SI EL MOTIVO ES FINISH ENVIO EL PROCESO A EXIT
            //SI EL RECURSO NO EXISTE ENVIO A EXIT
            //EN ESTE CASO EL MOTIVO NO PUEDE SER INTERRUMPT PORQUE ES FIFO
        }
        if(strcmp(algoritmo_planificacion, "RR")==0){
            //envio el primer elemento de la cola ready a EXEC
            t_PCB* retirar_ready = queue_pop(cola_ready);
            retirar_ready->quantum = quantum;
            queue_push(cola_exec, retirar_ready);
            //envio PCB a la CPU
            t_paquete* paquete_pcb = crear_paquete();
            t_paquete_entre* exec = malloc(sizeof(t_paquete_entre));
            //operacion
            exec->operacion=EXEC_PROCESO;
            //envio pcb
            exec->payload = retirar_ready;
            agregar_a_paquete(paquete_pcb, exec, sizeof(t_paquete_entre));
            enviar_paquete(paquete_pcb,resultHandshakeDispatch);
            eliminar_paquete(paquete_pcb);

            usleep(quantum);
            //interrumpo el proceso por fin de quantum
            t_paquete* paquete_fin_de_q = crear_paquete;
            t_paquete_entre* fin_q = malloc(sizeof(t_paquete_entre));
            fin_q->operacion = INTERRUMPIR_PROCESO;
            agregar_a_paquete(paquete_fin_de_q, fin_q, sizeof(t_paquete_entre));
            enviar_paquete(paquete_fin_de_q,resultHandshakeInterrupt);
            eliminar_paquete(paquete_fin_de_q);
            // espero recibir el pcb con motivo de desalojo 
            //SI EL MOTIVO ES WAIT VERIFICO SI EXISTE RECURSO SOLICITADO Y LE RESTO UNO, 
            // SI EL NUMERO DE RECURSO ES MENOR A 0 
            //BLOQUEO EL PROCESO CORRESPONDIENTE AL RECURSO

            //SI EL MOTIVO ES SIGNAL VERIFICAR QUE EXISTA, SUMARLE UNO, Y SACAR UN PROCESO DE LA COLA DE 
            //BLOQUEADOS
            //SI EL MOTIVO ES FINISH ENVIO EL PROCESO A EXIT
            //SI EL RECURSO NO EXISTE ENVIO A EXIT
            //SI ES INTERRUMPT VUELVO A ENCOLAR EL PROCESO EN READY
        }
    }
}


t_PCB* crear_PCB(int PID) {
    t_PCB* newPCB = malloc(sizeof(t_PCB));
    newPCB->PID = PID;
    newPCB->program_counter = 0;
    newPCB->cpu_registro.AX=0;
    newPCB->cpu_registro.BX=0;
    newPCB->cpu_registro.CX=0;
    newPCB->cpu_registro.DX=0;
    newPCB->cpu_registro.EAX=0;
    newPCB->cpu_registro.EBX=0;
    newPCB->cpu_registro.ECX=0;
    newPCB->cpu_registro.EDX=0;
    newPCB->cpu_registro.SI=0;
    newPCB->cpu_registro.DI=0;
    newPCB->quantum=0;
    newPCB->estado=NEW;
    return newPCB;
}
void iniciar_colas(void){
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_blocked = queue_create();
    cola_exec = queue_create();
    cola_exit = queue_create();
}

int g_multiprogracion_actual (void){
    return queue_size(cola_ready) + queue_size(cola_blocked) + queue_size(cola_exec);
}
