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
    // pthread_mutex_trylock(&logger_mutex);
    log_info(logger, "Iniciando consola");
    // pthread_mutex_unlock(&logger_mutex);

    while (1)
    {
        // pthread_mutex_trylock(&printf_mutex);
        printf("Ingrese un comando: \n");
        // pthread_mutex_unlock(&printf_mutex);

        // pthread_mutex_trylock(&consola_mutex);
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
            if (/*split[1]!= NULL && */strcmp(split[1], "") != 0 && strcmp(split[1], " ") != 0){
                ejecutar_script(split[1]);
                // free(split);
            }
            free(leido);
        }
        else if (string_equals_ignore_case(split[0], "FINALIZAR_PROCESO"))
        {
            printf("FINALIZANDO PROCESO");
            finalizar_proceso(atoi(split[1]));
            free(leido);
        }
        else if (string_equals_ignore_case(split[0], "DETENER_PLANIFICACION"))
        {
            detener_planificacion();
            free(leido);
        }
        else if (string_equals_ignore_case(split[0], "INICIAR_PLANIFICACION"))
        {
            iniciar_planificacion();
        free(leido);
        }
        else if (string_equals_ignore_case(split[0], "MULTIPROGRAMACION"))
        {
            modificar_multiprogramacion(atoi(split[1]));
            free(leido);
        }
        else if (string_equals_ignore_case(split[0], "PROCESO_ESTADO"))
        {
             listar_procesos_por_estado();
            free(leido);
        }
        else
        {
            // pthread_mutex_trylock(&printf_mutex);
            printf("Escribio un comando incorrecto, los comandos existentes son: \n EJECUTAR_SCRIPT \n INICIAR_PROCESO \n FINALIZAR_PROCESO \n DETENER_PLANIFICACION \n INICIAR_PLANIFICACION \nMULTIPROGRAMACION  \n PROCESO_ESTADO \n ");
            // pthread_mutex_unlock(&printf_mutex);
            free(leido);
            // printf("Escribio un comando incorrecto, los comandos existentes son: \n EJECUTAR_SCRIPT \n INICIAR_PROCESO \n FINALIZAR_PROCESO \n DETENER_PLANIFICACION \n INICIAR_PLANIFICACION \nMULTIPROGRAMACION  \n PROCESO_ESTADO \n ");
        }
    }
    return NULL;
}// EJECUTAR_SCRIPT ./scripts_kernel/prueba.txt
int ejecutar_script(char *path)
{
    // Abre el archivo

    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    char* path_instrucciones = "/scripts_kernel/";

    char* path_archivo = malloc(strlen(cwd) + strlen(path_instrucciones) + strlen(path) + 1);
    strcpy(path_archivo, cwd);
    strcat(path_archivo, path_instrucciones);
    strcat(path_archivo, path);

    log_info(logger, "%s", path_archivo);

    FILE *archivo = fopen(path_archivo, "r");
    if (!archivo)
    {
        log_error(logger, "Error abriendo el archivo %s", path);
        free(path);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), archivo))
    {
        // Eliminar el salto de línea al final de la línea si existe
        line[strcspn(line, "\n")] = 0;

        if (strncmp(line, "INICIAR_PROCESO ", 16) == 0)
        {
            iniciar_proceso(line + 16);
        }
        else if (strncmp(line, "FINALIZAR_PROCESO ", 18) == 0)
        {
            finalizar_proceso(atoi(line + 18));
        }
        else if (strcmp(line, "DETENER_PLANIFICACION") == 0)
        {
            detener_planificacion();
        }
        else if (strcmp(line, "INICIAR_PLANIFICACION") == 0)
        {
            iniciar_planificacion();
        }
        else if (strncmp(line, "MULTIPROGRAMACION ", 18) == 0)
        {
            modificar_multiprogramacion(atoi(line + 18));
        }
        else if (strcmp(line, "PROCESO_ESTADO") == 0)
        {
            listar_procesos_por_estado();
        }
        else
        {
            printf("Comando desconocido o malformado: %s\n", line);
        }
    }

    fclose(archivo);
    
    return 0;
}