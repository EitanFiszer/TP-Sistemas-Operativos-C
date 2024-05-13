#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h> 
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <main.h>
#include <kernel-utils/estados.h>
#include <kernel-utils/PCB.h>
#include <pthread.h>
#include <string.h>



int main(int argc, char *argv[])
{
    // decir_hola("Kernel");
    logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
    config = config_create("kernel.config");

    puerto = config_get_int_value(config, "PUERTO_ESCUCHA");

    char *stringParaLogger = string_from_format("[KERNEL] Escuchando en el puerto: %d", puerto);
    log_info(logger, stringParaLogger);

    // leemos las configs
    leer_configs();

    // cliente se conecta al sevidor
    // int resultHandshakeDispatch = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
    // int resultHandshakeInterrupt = connectAndHandshake(ip_cpu, puerto_cpu_interrupt, KERNEL, "cpu", logger);
    // int resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);

    // // creamos el servidor
    // int server_fd = iniciar_servidor(puerto_escucha, logger);
    // handshake_t res = esperar_cliente(server_fd, logger);
    // int modulo = res.modulo;
    // int socket_cliente = res.socket;
    // switch (modulo)
    // {
    // case IO:
    //     log_info(logger, "Se conecto un I/O");
    //     break;
    // default:
    //     log_error(logger, "Se conecto un cliente desconocido");
    //     break;
    // }
    //INICIO LAS COLAS 
    iniciar_colas();
    // creo hilo para que reciba informacion de la consola constantement
    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, consola_interactiva, NULL);
    // el hilo no necesita ser esperado por otro hilo que lo creo para liberar sus recursos cuando termine la ejecucion
    pthread_detach(hilo_consola);

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
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
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
            // leo el PATH
            char *instrucciones = split[1];
           
// int resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);
            // me conecto con la memoria

            //CREO EL PROCESO LOG
            log_info(logger,"Se crea el proceso <%d> en NEW", PID);
            int socket_cliente;
            socket_cliente = crear_conexion(ip_memoria,puerto_memoria);

            // creo el paquete con las instrucciones para enviar a memoria las instrucciones
            t_paquete* nuevo_paquete = crear_paquete();
            agregar_a_paquete(nuevo_paquete,instrucciones,(strlen(instrucciones)+1));
            //envio el identificador de proceso
            agregar_a_paquete(nuevo_paquete, PID,sizeof(int));
            //envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
            enviar_paquete(nuevo_paquete,socket_cliente);

            //creo la PCB Y la guardo en cola NEW
            PCB new_PCB=crear_PCB(PID);
            queue_push(cola_new,&new_PCB);

            //Incremento identificador de proceso
            PID++;

            free(split);
            free(leido);
            //elimino paquete
            eliminar_paquete(nuevo_paquete);
            //libero conexion 
            liberar_conexion(socket_cliente);
        }
    }
}

void LTS(void){
    while(1){
        //si el grado de multiprogramacion lo permite enviar procesos de new a ready
        int grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
        if (grado_multiprogramacion>0){
            PCB*  retirar_new = queue_pop(cola_new);
            queue_push(cola_ready,retirar_new);
        }
    }
}

void STS(void){
    while(1){
        char* algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
        if(strcmp(algoritmo_planificacion, "FIFO")==0){
            //envio el primer elemento de la cola ready a EXEC
            PCB* retirar_ready = queue_pop(cola_ready);
  
            queue_push(cola_exec, retirar_ready);

            //ENVIO PCB A CPU PARA Q LO EJECUTE Y QUEDO EN ESPERA DE SU CONTEXTO ACTUALIZADO
            int socket_cliente = crear_conexion(ip_cpu,puerto_cpu_dispatch);

            // creo el paquete con las instrucciones para enviar a cpu y la pcb
            t_paquete* nuevo_paquete = crear_paquete();
            //agre
            agregar_a_paquete(nuevo_paquete,retirar_ready,sizeof(PCB));
            //envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
            enviar_paquete(nuevo_paquete,socket_cliente);

            //espero la respuesta del proceso;
            int socket_servidor = iniciar_servidor(puerto, logger);

            handshake_t esperar_cpu = esperar_cliente(socket_servidor,logger);

            PCB* bloqueado = recibir_paquete(esperar_cpu.socket);

            



    // // creamos el servidor
    // int server_fd = iniciar_servidor(puerto_escucha, logger);
    // handshake_t res = esperar_cliente(server_fd, logger);
    // int modulo = res.modulo;
    // int socket_cliente = res.socket;
    // switch (modulo)
    // {
    // case IO:
    //     log_info(logger, "Se conecto un I/O");
    //     break;
    // default:
    //     log_error(logger, "Se conecto un cliente desconocido");
    //     break;
    // }
        }
    }
}

// char *leer_archivo(char *un_path)
// {
//     FILE *f = fopen(un_path, "r");
//     if (f == NULL)
//     {
//         return NULL;
//     }
//     fseek(f, 0, SEEK_END);
//     long int size = ftell(f);
//     rewind(f);
//     char *content = calloc(size + 1, 1);
//     fread(content, 1, size, f);
//     fclose(f);

//     return content;
// }
PCB crear_PCB(int PID) {
    PCB newPCB;
    newPCB.PID = PID;
    newPCB.quantum=0;
    newPCB.estado=NEW;
    return newPCB;
}
void iniciar_colas(void){
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_blocked = queue_create();
    cola_exec = queue_create();
    cola_exit = queue_create();
}