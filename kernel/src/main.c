#include <stdio.h>
#include "global.h"
#include <commons/config.h> 
#include <commons/log.h> 
#include "consola.h"
#include <pthread.h>
#include <espera.h>
#include "planificacion.h"


//conexiones

t_log *logger;
int puerto;
char* ip_memoria;
char* ip_cpu;
char* puerto_memoria;
char* puerto_escucha;
char* puerto_cpu_dispatch;
char* puerto_cpu_interrupt;
int resultHandshakeMemoria;
int resultHandshakeDispatch;
int resultHandshakeInterrupt;
//conexiones 
int server_fd;
int quantum;
char *algoritmo_planificacion; 


void leer_configs(t_config* config){
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    puerto = config_get_int_value(config, "PUERTO_ESCUCHA");
    quantum = config_get_int_value(config, "QUANTUM");
    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
}
void inicializar_recursos(){
    
    for (int i = 0; i < MAX_RECURSOS; i++) {
        strcpy(recursos[i].nombre_recurso, "NA"); // NA significa "No Asignado"
        recursos[i].instancias_recurso = 0;
        pthread_mutex_init(&recursos[i].mutex_recurso, NULL);
        queue_create(recursos[i].cola_blocked_recurso);
    }
}

void configurar_recurso(int index, char * nombre,int instancias){
    strncpy(recursos[index].nombre_recurso, nombre, 3);
    recursos[index].instancias_recurso = instancias;
}

void leer_recursos(t_config* config) {
    char** nombres_r = config_get_array_value(config,"RECURSOS");
    char** instancias_r = config_get_array_value(config, "INSTANCIAS_RECURSOS");
    if (nombres_r == NULL || instancias_r == NULL){
        log_info(logger,"ERROR AL LEER RECURSOS");
    }
    int i = 0;
    while (nombres_r[i]!=NULL && instancias_r[i] != NULL && i<MAX_RECURSOS)
    {
        int instancias = atoi (instancias_r);
        configurar_recurso(i,nombres_r[i],instancias);
        i++;
    }
    // Libera la memoria asignada por config_get_array_value
    i = 0;
    while (nombres_r[i] != NULL) {
        free(nombres_r[i]);
        i++;
    }
    free(nombres_r);
    i = 0;
    while (instancias_r[i] != NULL) {
        free(instancias_r[i]);
        i++;
    }
    free(instancias_r);
}

int main(){
    // decir_hola("Kernel");
    logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
    t_config *config = config_create("kernel.config");
    leer_configs(config);
    inicializar_recursos();
    leer_recursos(config);
    log_info(logger, "[KERNEL] Escuchando en el puerto: %d", puerto);

    // cliente se conecta al sevidor
    resultHandshakeDispatch = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
    resultHandshakeDispatch = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
    resultHandshakeInterrupt = connectAndHandshake(ip_cpu, puerto_cpu_interrupt, KERNEL, "cpu", logger);

    // ESTE ES EL SOCKET PARA CONECTARSE A LA MEMORIA
    resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);

    // creamos el servidor
    server_fd = iniciar_servidor(puerto_escucha, logger);

    handshake_t res = esperar_cliente(server_fd, logger);
    int modulo = res.modulo;
    //int socket_cliente = res.socket;
    switch (modulo)
    {
    case IO:
        log_info(logger, "Se conecto un I/O");
        break;
    default:
        log_error(logger, "Se conecto un cliente desconocido");
        break;
    }


    // creo hilo para que reciba informacion de la consola constantemente
    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, consola_interactiva, NULL);
    pthread_detach(hilo_consola);
    
    
    // HILO PARA QUE ESPERA PAQUETES DE LA CPU
    pthread_t hilo_espera_cpu;
    pthread_create(&hilo_espera_cpu, NULL, esperar_paquetes_cpu_dispatch, (void*)&resultHandshakeDispatch);
    pthread_detach(hilo_espera_cpu);

    // HILO PARA QUE ESPERE PAQUETES DE LA MEMORIA
    pthread_t hilo_espera_memoria;
    pthread_create(&hilo_espera_memoria, NULL, esperar_paquetes_memoria,(void*)&resultHandshakeMemoria);
    pthread_detach(hilo_espera_memoria);

    //HILO PARA MANEJAR PLANIFICACION 
    pthread_t hilo_planificacion;
    pthread_create(&hilo_planificacion, NULL, planificacion,NULL);
    pthread_detach(hilo_planificacion);

    return 0;
}