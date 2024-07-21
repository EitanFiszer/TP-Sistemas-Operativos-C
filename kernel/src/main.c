#include <stdio.h>
#include "global.h"
#include <commons/config.h>
#include <commons/log.h>
#include "consola.h"
#include <pthread.h>
#include <espera.h>
#include "planificacion.h"
#include <utils/iniciar.h>
#include "recursos.h"
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
int grado_multiprog;
// conexiones
int server_fd;
int64_t quantum;
char *algoritmo_planificacion;

pthread_mutex_t logger_mutex;
pthread_mutex_t printf_mutex;
pthread_mutex_t consola_mutex;
t_config *config;
t_dictionary* diccionario_recursos;


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
    grado_multiprog = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
}
void iniciar_mutex()
{
    pthread_mutex_init(&logger_mutex, NULL);
    pthread_mutex_init(&printf_mutex, NULL);
    pthread_mutex_init(&consola_mutex, NULL);
}



void esperar_clientes_io(void *args)
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
            pthread_create(&hilo_atender_cliente, NULL, (void *)atender_cliente, &fd_conexion_ptr);
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
    logger = iniciar_logger_kernel("kernel.log", "Kernel");
    config = iniciar_config("kernel.config");
    leer_configs(config);
    // inicializar_recursos();
    // leer_recursos(config);
    log_info(logger, "[KERNEL] Escuchando en el puerto: %s", puerto_escucha);

    // inicializar_rec_pid();
    guardar_dictionary_recursos(config);
    inicializar_interfaces();
    inicializar_operaciones_en_espera();

    //probando recursos
    // t_recurso_reg* unRecurso = dictionary_get(diccionario_recursos,"RA");
    // log_info(logger,"Un nombre del diccionario : %s, con instancias: %d",unRecurso->nombre_recurso, unRecurso->instancias_recurso);


    // cliente se conecta al sevidor
    

    resultHandshakeDispatch = connectAndHandshake(ip_cpu, puerto_cpu_dispatch, KERNEL, "cpu", logger);
    resultHandshakeInterrupt = connectAndHandshake(ip_cpu, puerto_cpu_interrupt, KERNEL, "cpu", logger);

    // // ESTE ES EL SOCKET PARA CONECTARSE A LA MEMORIA
    resultHandshakeMemoria = connectAndHandshake(ip_memoria, puerto_memoria, KERNEL, "memoria", logger);

    // // creamos el servidor
    server_fd = iniciar_servidor(puerto_escucha, logger);

   

    // HILO PARA ATENDER CLIENTES -- ESPERAR QUE SE CONECTEN IO
    pthread_t hilo_esperar_clientes;
    int err = pthread_create(&hilo_esperar_clientes, NULL, (void *)esperar_clientes_io, NULL);
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