#include <stdio.h>
#include "global.h"
#include <commons/config.h>
#include <commons/log.h>
#include "consola.h"
#include <pthread.h>
#include <espera.h>
#include "planificacion.h"
#include <utils/iniciar.h>
// #include <utils/client.h>
// #include <utils/server.h>

// conexiones

t_log *logger;
// int puerto;
char *ip_memoria;
char *ip_cpu;
char *puerto_memoria;
char *puerto_escucha;
char *puerto_cpu_dispatch;
char *puerto_cpu_interrupt;
int resultHandshakeMemoria;
int resultHandshakeDispatch;
int resultHandshakeInterrupt;
// conexiones
int server_fd;
int64_t quantum;
char *algoritmo_planificacion;

pthread_mutex_t logger_mutex;
pthread_mutex_t printf_mutex;
pthread_mutex_t consola_mutex;

void leer_configs(t_config *config)
{
    ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    ip_cpu = config_get_string_value(config, "IP_CPU");
    puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    puerto_cpu_dispatch = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    puerto_cpu_interrupt = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    quantum = config_get_int_value(config, "QUANTUM");
    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
}
void iniciar_mutex()
{
    pthread_mutex_init(&logger_mutex, NULL);
    pthread_mutex_init(&printf_mutex, NULL);
    pthread_mutex_init(&consola_mutex, NULL);
}
// void inicializar_recursos(){

//     for (int i = 0; i < MAX_RECURSOS; i++) {
//         strcpy(recursos[i].nombre_recurso, "NA"); // NA significa "No Asignado"
//         recursos[i].instancias_recurso = 0;
//         pthread_mutex_init(&recursos[i].mutex_recurso, NULL);
//         queue_create(recursos[i].cola_blocked_recurso);
//     }
// }

// void configurar_recurso(int index, char * nombre,int instancias){
//     strncpy(recursos[index].nombre_recurso, nombre, 3);
//     recursos[index].instancias_recurso = instancias;
// }

// void leer_recursos(t_config* config) {
//     char** nombres_r = config_get_array_value(config,"RECURSOS");
//     char** instancias_r = config_get_array_value(config, "INSTANCIAS_RECURSOS");
//     if (nombres_r == NULL || instancias_r == NULL){
//         log_info(logger,"ERROR AL LEER RECURSOS");
//     }
//     int i = 0;
//     while (nombres_r[i]!=NULL && instancias_r[i] != NULL && i<MAX_RECURSOS)
//     {
//         int instancias = atoi (instancias_r[i]);
//         configurar_recurso(i,nombres_r[i],instancias);
//         i++;
//     }
//     // Libera la memoria asignada por config_get_array_value
//     i = 0;
//     while (nombres_r[i] != NULL) {
//         free(nombres_r[i]);
//         i++;
//     }
//     free(nombres_r);
//     i = 0;
//     while (instancias_r[i] != NULL) {
//         free(instancias_r[i]);
//         i++;
//     }
//     free(instancias_r);
// }

void esperar_clientes_kernel(void *args)
{
    while (1)
    {
        pthread_t hilo_atender_cliente;
        int fd_conexion_ptr = -1;
        Handshake res = esperar_cliente(server_fd, logger);
        int modulo = res.modulo;
        fd_conexion_ptr = res.socket;
        switch (modulo)
        {
        case IO:
            log_info(logger, "Se conecto un I/O");
            pthread_create(&hilo_atender_cliente, NULL, (void *)atender_cliente, fd_conexion_ptr);
            pthread_detach(hilo_atender_cliente);
            break;
        default:
            log_error(logger, "Se conecto un cliente desconocido");
            break;
        }
    }
}

int main()
{
    // decir_hola("Kernel");
    logger = iniciar_logger("kernel.log", "Kernel");
    t_config *config = iniciar_config("kernel.config");
    leer_configs(config);
    // inicializar_recursos();
    // leer_recursos(config);
    log_info(logger, "[KERNEL] Escuchando en el puerto: %s", puerto_escucha);

    // cliente se conecta al sevidor
    // resultHandshakeDispatch = conectarse_cpu_dispatch(ip_cpu,puerto_cpu_dispatch);
    // resultHandshakeDispatch = conectarse_cpu_interrupt(ip_cpu,puerto_cpu_interrupt);
    // resultHandshakeDispatch = conectarse_memoria(ip_cpu,puerto_memoria);

    resultHandshakeDispatch = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
    resultHandshakeInterrupt = connectAndHandshake(ip_cpu, puerto_cpu_interrupt, KERNEL, "cpu", logger);

    // // ESTE ES EL SOCKET PARA CONECTARSE A LA MEMORIA
    resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);

    // // creamos el servidor
    server_fd = iniciar_servidor(puerto_escucha, logger);

    // Handshake res = esperar_cliente(server_fd, logger);
    // int modulo = res.modulo;
    // // int socket_cliente = res.socket;
    // switch (modulo)
    // {
    // case IO:
    //     log_info(logger, "Se conecto un I/O");
    //     break;
    // default:
    //     log_error(logger, "Se conecto un cliente desconocido");
    //     break;
    // }

    // HILO PARA ATENDER CLIENTES -- ESPERAR QUE SE CONECTEN IO
    pthread_t hilo_esperar_clientes;
    int err = pthread_create(&hilo_esperar_clientes, NULL, (void *)esperar_clientes_kernel, NULL);
    pthread_detach(hilo_esperar_clientes);
    if (err != 0)
    {
        log_error(logger, "HUBO UN ERROR AL CREAR EL HILO");
    }

    // HILO PARA QUE ESPERA PAQUETES DE LA CPU
    pthread_t hilo_espera_cpu;
    err = pthread_create(&hilo_espera_cpu, NULL, esperar_paquetes_cpu_dispatch, NULL);
    pthread_detach(hilo_espera_cpu);
    if (err != 0)
    {
        log_error(logger, "HUBO UN ERROR AL CREAR EL HILO");
    }
    // HILO PARA QUE ESPERE PAQUETES DE LA MEMORIA
    pthread_t hilo_espera_memoria;
    err = pthread_create(&hilo_espera_memoria, NULL, esperar_paquetes_memoria, NULL);
    pthread_detach(hilo_espera_memoria);
    if (err != 0)
    {
        log_error(logger, "HUBO UN ERROR AL CREAR EL HILO");
    }
    // HILO PARA MANEJAR PLANIFICACION
    pthread_t hilo_planificacion;
    err = pthread_create(&hilo_planificacion, NULL, planificacion, NULL);
    pthread_detach(hilo_planificacion);
    if (err != 0)
    {
        log_error(logger, "HUBO UN ERROR AL CREAR EL HILO");
    }
    // creo hilo para que reciba informacion de la consola constantemente
    pthread_t hilo_consola;
    err = pthread_create(&hilo_consola, NULL, consola_interactiva, NULL);
    pthread_join(hilo_consola, NULL);
    if (err != 0)
    {
        log_error(logger, "HUBO UN ERROR AL CREAR EL HILO");
    }
    return 0;
}