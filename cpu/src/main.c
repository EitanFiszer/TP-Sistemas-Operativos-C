#include <utils/client.h>
#include <cpu-utils/conexiones.h>
#include <cpu-utils/cicloInstruccion.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <pthread.h>

void finalizarCPU (t_log* logger, t_config* config) {
    log_destroy(logger);
    config_destroy(config);
    exit(1);
}

registros_t registros;

int main(int argc, char* argv[]) {
    // creamos logs y configs
    t_log* logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("cpu.config");

    // leemos las configs
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    char* puerto_escucha_dispatch = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    char* puerto_escucha_interrupt = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");

    log_debug(logger, "Configuraciones leidas %s, %s, %s, %s", ip_memoria, puerto_memoria, puerto_escucha_dispatch, puerto_escucha_interrupt);

    // declaramos los hilos
    pthread_t hilo_interrupt;
    args argumentos_interrupt;
    argumentos_interrupt.puerto = puerto_escucha_interrupt;
    argumentos_interrupt.logger = logger;
    
    pthread_create(&hilo_interrupt, NULL, conexion_interrupt, (void*)&argumentos_interrupt);
    pthread_detach(hilo_interrupt);

    //El cliente se conecta 
    int socketMemoria = connectAndHandshake(ip_memoria, puerto_memoria, CPU, "memoria", logger);
    if (socketMemoria == -1) {
        log_error(logger, "No se pudo conectar con la memoria");
        finalizarCPU(logger,config);
    }
    printf("Handshake socket: %d\n", socketMemoria);

    while (1) {
        // Recibo el paquete de la memoria
        t_list* paquetePCB = recibir_paquete(socketMemoria);
        if (paquetePCB == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la memoria");
            finalizarCPU(logger, config);
        } 
        
        t_PCB* pcb = list_get(paquetePCB, 0);

        // Hago el fetch de la instruccion
        char* instruccionRecibida;
        int ok = fetchInstruccion(pcb, socketMemoria, &instruccionRecibida, logger);

        if (ok == -1) {
            log_error(logger, "No se pudo recibir la instruccion de la memoria");
            finalizarCPU(logger, config);
        }

        // Decodifico la instruccion
        instruccionCPU_t* instruccion = dividirInstruccion(instruccionRecibida);

        // Ejecuto la instruccion
        ejecutarInstruccion(instruccion, pcb, socketMemoria, logger, &registros);

        // Devolver el PCB al kernel
        // enviar_PCB(pcb, socketKernel);
        
    }

    finalizarCPU(logger, config);
}