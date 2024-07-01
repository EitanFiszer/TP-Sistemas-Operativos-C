#include "consola.h"

// pthread_mutex_t logger_mutex;
// pthread_mutex_t printf_mutex;
// pthread_mutex_t consola_mutex;

// t_PCB *unaPCB = crear_PCB();

// 	t_paquete_entre *paquete_entre = malloc(sizeof(t_paquete_entre));
// 	paquete_entre->operacion = EXEC_PROCESO;
// 	paquete_entre->size_payload = sizeof(t_PCB);
// 	paquete_entre->payload = unaPCB;

// 	t_paquete *paquete = crear_paquete();
// 	// agregar_paquete_entre_a_paquete(paquete, paquete_entre,sizeof(t_PCB));
// 	agregar_paquete_entre_a_paquete(paquete, paquete_entre);
// 	enviar_paquete(paquete, conexion);
// 	eliminar_paquete(paquete);

// //ENVIO UN PID	
// 	int PID = 5;
// 	paquete_entre->operacion = FINALIZAR_PROCESO;
// 	paquete_entre->size_payload = sizeof(int);
// 	paquete_entre->payload = &PID;

// 	t_paquete *otro_paquete = crear_paquete();
// 	agregar_paquete_entre_a_paquete(otro_paquete, paquete_entre);
// 	enviar_paquete(paquete, conexion);

// 	free(otro_paquete);
// 	free(paquete_entre);

void *consola_interactiva()
{
    // pthread_mutex_lock(&logger_mutex);
    log_info(logger, "Iniciando consola");
    // pthread_mutex_unlock(&logger_mutex);

    while (1)
    {
        // pthread_mutex_lock(&printf_mutex);
        printf("Ingrese un comando: \n");
        // pthread_mutex_unlock(&printf_mutex);

        // pthread_mutex_lock(&consola_mutex);
        char *leido = readline(">");

        char **split = string_split(leido, " ");
        // int length = string_array_size(split);
        // pthread_mutex_unlock(&consola_mutex);

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
            // pthread_mutex_lock(&printf_mutex);
            printf("Escribio un comando incorrecto, los comandos existentes son: \n EJECUTAR_SCRIPT \n INICIAR_PROCESO \n FINALIZAR_PROCESO \n DETENER_PLANIFICACION \n INICIAR_PLANIFICACION \nMULTIPROGRAMACION  \n PROCESO_ESTADO \n ");
            // pthread_mutex_unlock(&printf_mutex);

            // printf("Escribio un comando incorrecto, los comandos existentes son: \n EJECUTAR_SCRIPT \n INICIAR_PROCESO \n FINALIZAR_PROCESO \n DETENER_PLANIFICACION \n INICIAR_PLANIFICACION \nMULTIPROGRAMACION  \n PROCESO_ESTADO \n ");
        }
    }
    return NULL;
}