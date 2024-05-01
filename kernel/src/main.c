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

    int puerto = config_get_int_value(config, "PUERTO_ESCUCHA");

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

    // creo hilo para que reciba informacion de la consola constantemente

    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, consola_interactiva, NULL);
    // el hilo no necesita ser esperado por otro hilo que lo creo para liberar sus recursos cuando termine la ejecucion
    pthread_detach(hilo_consola);

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
            char *instrucciones = leer_archivo(split[1]);
            // creo la PCB Y SE LA ENVIO A LA CPU
            PCB nuevo_proceso;
            nuevo_proceso.estado = NEW;
            nuevo_proceso.PID = PID;
            // incremento el identificador del proceso para el proximo
            PID++;
            /// faltaria inicializar registro de la cpu y program counter quantum segun sea RR o FIFO

            // me conecto con la cpu ----NO USE HANDSHAKE----
            //CREO EL PROCESO LOG
            log_info(logger,"Se crea el proceso <%d> en NEW", nuevo_proceso.PID);
            int socket_cliente;
            socket_cliente = crear_conexion(ip_cpu,puerto_cpu_dispatch);

            // creo el paquete con las instrucciones para enviar a cpu y la pcb
            t_paquete* nuevo_paquete = crear_paquete();
            agregar_a_paquete(nuevo_paquete,instrucciones,(strlen(instrucciones)+1));
            agregar_a_paquete(nuevo_paquete, &nuevo_proceso,sizeof(PCB));


            //envio el paquete a la CPU //ENVIO EL NUEVO PROCESO
            enviar_paquete(nuevo_paquete,socket_cliente);
            

            free(split);
            free(leido);
            //elimino paquete
            eliminar_paquete(nuevo_paquete);
            //libero conexion 
            liberar_conexion(socket_cliente);
        }
    }
}
char *leer_archivo(char *un_path)
{
    FILE *f = fopen(un_path, "r");
    if (f == NULL)
    {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long int size = ftell(f);
    rewind(f);
    char *content = calloc(size + 1, 1);
    fread(content, 1, size, f);
    fclose(f);

    return content;
}
void crear_PCB(void) {}
/*void iniciar_colas(){
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_blocked = queue_create();
    cola_exec = queue_create();
    cola_exit = queue_create();
}*/
//
