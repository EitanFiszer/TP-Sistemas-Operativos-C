#include "main.h"
#include <utils/iniciar.h>

void finalizarCPU (t_log* logger, t_config* config) {
    printf("Finalizando CPU\n");
    log_destroy(logger);
    config_destroy(config);
    liberar_conexion(socketMemoria);
    // liberar_conexion(socketKernel);
    exit(1);
}

registros_t registros;
int socketKernel;
int socketMemoria;
int TAM_PAGINA;
t_log* logger;
int interrupcion;

t_list* TLB;
int TLB_MAX_SIZE;
tlb_reemplazo TLB_ALGORITMO_REEMPLAZO;

pthread_mutex_t mutex_interrupcion;

struct args {
    char* puerto;
    t_log* logger;
};

void conexion_interrupt(void *argumentos) {
    struct args *puertoYLogger = argumentos;
    char* puerto = puertoYLogger->puerto;
    t_log* logger = puertoYLogger->logger;
    // creamos el servidor
    int server_interrupt_fd = iniciar_servidor(puerto, logger);
    log_info(logger, "[CPU] Escuchando en el puerto interrupt: %s", puerto);


    Handshake res = esperar_cliente(server_interrupt_fd, logger);
    int modulo = res.modulo;
    int socket_cliente = res.socket;
    switch (modulo) {
	    case KERNEL:
		    log_info(logger, "Se conecto un Kernel");
    	break;
        default:
		    log_error(logger, "Se conecto un cliente desconocido");
		break;
    }

    while(1) {
        
        t_paquete_entre* paquete = recibir_paquete_entre(socket_cliente);
        
        switch (paquete->operacion) {
            case INTERRUMPIR_PROCESO:
                pthread_mutex_lock(&mutex_interrupcion);
                interrupcion = true;
                pthread_mutex_unlock(&mutex_interrupcion);
                break;
            default:
                log_error(logger, "Operacion desconocida");
                break;
        }
    }
}

int getHayInterrupcion() {
    pthread_mutex_lock(&mutex_interrupcion);
    int hayInterrupcion = interrupcion;
    pthread_mutex_unlock(&mutex_interrupcion);
    return hayInterrupcion;
}

// TESTS
void testConnMem() {
    OP_CODES_ENTRE op = PC_A_INSTRUCCION;
    t_payload_pc_a_instruccion* payload = malloc(sizeof(t_payload_pc_a_instruccion));
    payload->PID = 6;
    payload->program_counter = 9;
    t_paquete* paq = crear_paquete();
    t_paquete_entre* paq_entre = malloc(sizeof(t_paquete_entre));

    paq_entre->operacion = op;
    paq_entre->size_payload = sizeof(t_payload_pc_a_instruccion);
    paq_entre->payload = payload;

    agregar_paquete_entre_a_paquete(paq, paq_entre);
    enviar_paquete(paq, socketMemoria);
    eliminar(paq);
    free(payload);
}


int main(int argc, char* argv[]) {
    // creamos logs y configs
    logger = iniciar_logger("cpu.log", "CPU");
    t_config* config = iniciar_config("cpu.config");
    
    // leemos las configs
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    char* puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    TLB_MAX_SIZE = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    TLB_ALGORITMO_REEMPLAZO = (tlb_reemplazo)config_get_string_value(config, "ALGORITMO_TLB");

    log_debug(logger, "Configuraciones leidas %s, %s, %s, %s", ip_memoria, puerto_memoria, puerto_escucha_dispatch, puerto_escucha_interrupt);


    pthread_mutex_init(&mutex_interrupcion,NULL);

    // declaramos los hilos
    pthread_t hilo_interrupt;
    args argumentos_interrupt;
    argumentos_interrupt.puerto = puerto_escucha_interrupt;
    argumentos_interrupt.logger = logger;
    
    pthread_create(&hilo_interrupt, NULL, (void*)conexion_interrupt, (void*)&argumentos_interrupt);
    pthread_detach(hilo_interrupt);

    // inicializamos la TLB
    TLB = list_create();

    //El cliente se conecta 
    handshake_cpu_memoria handshakeMemoria = handshake_memoria(ip_memoria, puerto_memoria);
    socketMemoria = handshakeMemoria.socket;
    TAM_PAGINA = handshakeMemoria.tam_pagina;
    if (socketMemoria == -1) {
        log_error(logger, "No se pudo conectar con la memoria");
        finalizarCPU(logger,config);
    }
    printf("Handshake socket: %d, TAM_PAG: %d\n", socketMemoria, TAM_PAGINA);

   

    // El kernel se conecta a nosotros (CPU) y recibimos su handshake para poder recibir el pcb de parte del kernel
    int server_dispatch_fd = iniciar_servidor(puerto_escucha_dispatch, logger);
     log_info(logger, "CPU listo para recibir al cliente");
     socketKernel = -1;
     while (socketKernel == -1){
         Handshake handshake_res = esperar_cliente(server_dispatch_fd, logger);
         if (handshake_res.modulo == KERNEL){
             socketKernel = handshake_res.socket;
             log_info(logger, "Se conecto un Kernel");
             break;
         } else {
             log_error(logger, "Se conecto un socket desconocido");
         }
     }


     while (1) {
         // Recibo el paquete del kernel
         

         t_paquete_entre* paq = recibir_paquete_entre(socketKernel);
         if (paq == NULL) {
             log_error(logger, "No se pudo recibir el paquete de la memoria");
         }
         t_PCB* pcb = (t_PCB*)paq->payload;
         switch (paq->operacion) {
             case EXEC_PROCESO:
                 while(getHayInterrupcion() == false) {
                     char* instruccionRecibida;
                     int ok = fetchInstruccion(pcb, socketMemoria, &instruccionRecibida, logger);

                     if (ok == -1) {
                         log_error(logger, "PROCESO TERMINÓ EJECUCIÓN: PID %d", pcb->PID);
                         // Devolver el PCB al kernel
                         enviar_pcb_kernel(pcb, TERMINO_EJECUCION);
                         break;
                     }

                     // Decodifico la instruccion en opcode y parametros
                     instruccionCPU_t* instruccion = dividirInstruccion(instruccionRecibida);

                     // Ejecuto la instruccion
                     ejecutarInstruccion(instruccion, pcb, logger, registros, socketKernel);
                 }
                 if (interrupcion) {
                     log_info(logger, "Interrupcion recibida");

                     pthread_mutex_lock(&mutex_interrupcion);
                     interrupcion = false;
                     pthread_mutex_unlock(&mutex_interrupcion);
                     // Interrumpir el proceso
                     enviar_pcb_kernel(pcb, INTERRUMPIO_PROCESO);
                 }
                 break;
             default:
                 log_error(logger, "Operacion desconocida");
                 break; 
         }

     } // El kernel se conecta a nosotros (CPU) y recibimos su handshake para poder recibir el pcb de parte del kernel
     int server_dispatch_fd = iniciar_servidor(puerto_escucha_dispatch, logger);
     log_info(logger, "CPU listo para recibir al cliente");
     socketKernel = -1;
     while (socketKernel == -1){
         Handshake handshake_res = esperar_cliente(server_dispatch_fd, logger);
         if (handshake_res.modulo == KERNEL){
             socketKernel = handshake_res.socket;
             log_info(logger, "Se conecto un Kernel");
             break;
         } else {
             log_error(logger, "Se conecto un socket desconocido");
         }
     }


     while (1) {
        
         t_paquete_entre* paq = recibir_paquete_entre(socketKernel);
         if (paq == NULL) {
             log_error(logger, "No se pudo recibir el paquete de la memoria");
         }
         t_PCB* pcb = (t_PCB*)paq->payload;

         switch (paq->operacion) {
             case EXEC_PROCESO:
                 while(getHayInterrupcion() == false) {
                     char* instruccionRecibida;
                     int ok = fetchInstruccion(pcb, socketMemoria, &instruccionRecibida, logger);

                     if (ok == -1) {
                         log_error(logger, "PROCESO TERMINÓ EJECUCIÓN: PID %d", pcb->PID);
                         // Devolver el PCB al kernel
                         enviar_pcb_kernel(pcb, TERMINO_EJECUCION);
                         break;
                     }

                     // Decodifico la instruccion en opcode y parametros
                     instruccionCPU_t* instruccion = dividirInstruccion(instruccionRecibida);

                     // Ejecuto la instruccion
                     ejecutarInstruccion(instruccion, pcb, logger, registros, socketKernel);
                 }
                 if (interrupcion) {
                     log_info(logger, "Interrupcion recibida");

                     pthread_mutex_lock(&mutex_interrupcion);
                     interrupcion = false;
                     pthread_mutex_unlock(&mutex_interrupcion);
                     // Interrumpir el proceso
                     enviar_pcb_kernel(pcb, INTERRUMPIO_PROCESO);
                 }
                 break;
             default:
                 log_error(logger, "Operacion desconocida");
                 break; 
         }

     }
    
    finalizarCPU(logger, config);
    
}