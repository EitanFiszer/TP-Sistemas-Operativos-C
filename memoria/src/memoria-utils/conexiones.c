#include "./conexiones.h"
#include <commons/log.h>
#include <memoria-utils/procesos.h>
#include <semaphore.h>

extern t_log *logger;
extern char *path_instrucciones;
extern sem_t sem_kernel;
extern sem_t sem_cpu;
extern int socketKernel;
extern int socketCpu;

void esperar_paquetes_kernel()
{
    sem_wait(&sem_kernel);
    printf("Esperando paquetes de KERNEL en el socket %d\n", socketKernel);
    while (1) {
        t_list *paquete = recibir_paquete(socketKernel);
        t_paquete_entre *paquete_kernel = list_get(paquete, 0);

        switch (paquete_kernel->operacion) {
            case CREAR_PROCESO:
                t_payload_crear_proceso *payloadCrear = paquete_kernel->payload;
                int pidCrear = payloadCrear->pid;
                char *path = payloadCrear->path;
                log_info(logger, "Se llamó a CREAR_PROCESO con PID: %d, archivo: %s", pidCrear, path);
                crearProceso(path, pidCrear);
                break;
            case FINALIZAR_PROCESO: 
                t_payload_finalizar_proceso *payloadFin = paquete_kernel->payload;
                int pidFin = payloadFin->pid;
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
    sem_wait(&sem_cpu);    
    printf("Esperando paquetes de CPU en el socket %d\n", socketCpu);
    while (1) {
        t_list *paquete = recibir_paquete(socketCpu);

        if (paquete == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la CPU");
        } else {
            printf("Paquete recibido de CPU, %d\n", list_size(paquete));
        }

        t_paquete_entre *paquete_cpu = list_get(paquete, 0);

        switch (paquete_cpu->operacion) {
            case PC_A_INSTRUCCION:
                t_payload_pc_a_instruccion *payload = paquete_cpu->payload;
                int pid = payload->PID;
                int pc = payload->program_counter;
                log_info(logger, "Se llamó a PC_A_INSTRUCCION para PID: %d con PC: %d", pid, pc);
                char *instruccion = obtenerInstruccion(pid, pc);

                t_paquete *respuesta = crear_paquete();
                t_paquete_entre *instruccion_respuesta = malloc(sizeof(t_paquete_entre));
                // Definir si incluímos un atributo "operación" = INSTRUCCION
                instruccion_respuesta->payload = instruccion;
                agregar_a_paquete(respuesta, instruccion_respuesta, sizeof(t_paquete_entre));
                enviar_paquete(respuesta, socketCpu);
                eliminar_paquete(respuesta);
                break;
            default:
                log_info(logger, "Operación desconocida de CPU");
                break;
        }

        // eliminar_paquete(paquete_cpu);
    }
}
