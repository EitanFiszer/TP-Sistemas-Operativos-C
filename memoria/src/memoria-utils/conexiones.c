#include "./conexiones.h"
#include <commons/log.h>
#include <memoria-utils/procesos.h>

void esperar_paquetes_kernel(int socketKernel, t_log* logger, char* path_instrucciones) {
    while (1) {
        t_list* paquete = recibir_paquete(socketKernel);
        t_paquete_entre* paquete_kernel = list_get(paquete, 0);
        
        switch (paquete_kernel->operacion) {
        case CREAR_PROCESO:
            {
                t_payload_crear_proceso* payload = paquete_kernel->payload;
                int pid = payload->pid;
                char* path = payload->path;
                log_info(logger, "Se llamó a CREAR_PROCESO con PID: %d, archivo: %s", pid, path);
                crearProceso(path_instrucciones, pid, logger);
            }
            break;
        case FINALIZAR_PROCESO:
            {
                t_payload_finalizar_proceso* payload = paquete_kernel->payload;
                int pid = payload->pid;
                log_info(logger, "Se llamó a FINALIZAR_PROCESO con PID: %d", pid);
                finalizarProceso(pid, logger);
            }
            break;
        default:
            log_info(logger, "Operación desconocida de KERNEL");
            break;
        }
        
        // eliminar_paquete(paquete_kernel);
    }
}

void esperar_paquetes_cpu(int socketCpu, t_log* logger) {
    while (1) {
        t_list* paquete = recibir_paquete(socketCpu);
        t_paquete_entre* paquete_cpu = list_get(paquete, 0);
        
        switch (paquete_cpu->operacion) {
        case PC_A_INSTRUCCION:
            {
                t_payload_pc_a_instruccion* payload = paquete_cpu->payload;
                int pid = payload->PID;
                int pc = payload->program_counter;
                log_info(logger, "Se llamó a PC_A_INSTRUCCION para PID: %d con PC: %d", pid, pc);
                char* instruccion = obtenerInstruccion(pid, pc);
                
                t_paquete* respuesta = crear_paquete();
                t_paquete_entre* instruccion_respuesta = malloc(sizeof(t_paquete_entre));
                //Definir si incluímos un atributo "operación" = INSTRUCCION
                instruccion_respuesta->payload = instruccion;
                agregar_a_paquete(respuesta, instruccion_respuesta, sizeof(t_paquete_entre));
                enviar_paquete(respuesta, socketCpu);
                eliminar_paquete(respuesta);
            }
            break;
        default:
            log_info(logger, "Operación desconocida de CPU");
            break;
        }

        // eliminar_paquete(paquete_cpu);
    }
}
