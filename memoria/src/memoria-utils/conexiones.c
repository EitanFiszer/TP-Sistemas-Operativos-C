#include "./conexiones.h"
#include <commons/log.h>
#include <utils/serializacion.h>
#include <utils/envios.h>
#include <memoria-utils/procesos.h>
#include <semaphore.h>

extern t_log *logger;
extern char *path_instrucciones;
extern int retardo_respuesta;
extern sem_t sem_kernel;
extern sem_t sem_cpu;
extern int socketKernel;
extern int socketCpu;


void esperar_paquetes_kernel()
{
    sem_wait(&sem_kernel);
    printf("Esperando paquetes de KERNEL en el socket %d\n", socketKernel);

    while (1) {
        t_paquete_entre *paquete = recibir_paquete_entre(socketKernel);

        switch (paquete->operacion) {
            case CREAR_PROCESO:
                t_payload_crear_proceso *payloadCrear = deserializar_crear_proceso(paquete->payload);
                int pidCrear = payloadCrear->pid;
                char *path = payloadCrear->path;
                log_info(logger, "Se llamó a CREAR_PROCESO con PID: %d, archivo: %s", pidCrear, path);
                crearProceso(path, pidCrear);
                break;
            case FINALIZAR_PROCESO: 
                int *payloadFin = (int*) paquete->payload;
                int pidFin = *payloadFin;
                log_info(logger, "Se llamó a FINALIZAR_PROCESO con PID: %d", pidFin);
                finalizarProceso(pidFin);
                break;
            default:
                log_info(logger, "Operación desconocida de KERNEL");
                break;
        }

        // eliminar_paquete(paquete_kernel);
    }
}

void esperar_paquetes_cpu()
{
    log_info(logger, "Esperar_paq_cpu");
    sem_wait(&sem_cpu);    
    log_info(logger,"Esperando paquetes de CPU en el socket %d\n", socketCpu);
    while (1) {
        printf("Entró en while\n");
        t_paquete_entre *paquete_cpu = recibir_paquete_entre(socketCpu);

        if (paquete_cpu == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la CPU, cerrando hilo");
            break;
        } 
        // else {
        //     log_info(logger,"Paquete recibido de CPU, %d, %d\n", paquete_cpu->operacion, paquete_cpu->size_payload);
        // }

        if (paquete_cpu->payload == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la CPU");
        } 
        // else {
        //     log_info(logger,"Paquete recibido de CPU, %d, %d\n", paquete_cpu->operacion, paquete_cpu->size_payload);
        // }

        log_info(logger,"%d",paquete_cpu->operacion);

        switch (paquete_cpu->operacion) {
            case PC_A_INSTRUCCION:
                usleep(retardo_respuesta * 1000);

                // Recibir PID y PC
                t_payload_pc_a_instruccion *payload = (t_payload_pc_a_instruccion *)paquete_cpu->payload;
                int pid = payload->PID;
                int pc = payload->program_counter;
                log_info(logger, "Se llamó a PC_A_INSTRUCCION para PID: %d con PC: %d", pid, pc);


                // Obtener instrucción
                char *instruccion = obtenerInstruccion(pid, pc);
                printf("Instruccion: %s\n", instruccion);

                if (instruccion == NULL) {
                    log_info(logger, "Fin de archivo");
                    enviar_paquete_entre(socketCpu, FIN_DE_INSTRUCCIONES, NULL, 0);
                    break;
                }

                // Enviar instrucción a CPU
                t_payload_get_instruccion* payloadGet = malloc(sizeof(t_payload_get_instruccion));
                payloadGet->instruccion = instruccion;
                printf("InstruccionGet: %s\n", payloadGet->instruccion);

                int size_instruccion;
                void* instruccionSerializada = serializar_get_instruccion(payloadGet, &size_instruccion);
                enviar_paquete_entre(socketCpu, GET_INSTRUCCION, instruccionSerializada, size_instruccion);
                break;
            default:
                log_info(logger, "Operación desconocida de CPU");
                break;
        }

        // eliminar_paquete(paquete_cpu);
    }
}
