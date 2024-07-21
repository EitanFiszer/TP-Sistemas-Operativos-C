#include "./conexiones.h"
#include <commons/log.h>
#include <utils/serializacion.h>
#include <utils/envios.h>
#include <memoria-utils/procesos.h>
#include <memoria-utils/memoria.h>
#include <semaphore.h>

extern t_log *logger;
extern char *path_instrucciones;
extern int retardo_respuesta;
extern sem_t sem_kernel;
extern sem_t sem_cpu;
extern sem_t sem_io;
extern int socketKernel;
extern int socketCpu;
extern int socketIO;
extern int server_fd;


void esperar_paquetes_kernel() {
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
                finalizarProceso(pidFin);
                break;
            default:
                log_info(logger, "Operación desconocida de KERNEL");
                break;
        }
    }
    liberarMemoria();
}

void esperar_paquetes_cpu() {
    sem_wait(&sem_cpu);    
    log_info(logger,"Esperando paquetes de CPU en el socket %d\n", socketCpu);
    while (1) {
        t_paquete_entre *paquete_cpu = recibir_paquete_entre(socketCpu);

        if (paquete_cpu == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la CPU, cerrando hilo");
            liberarMemoria();
            break;
        } 

        if (paquete_cpu->payload == NULL) {
            log_error(logger, "No se pudo recibir el paquete de la CPU");
            liberarMemoria();
        } 

        switch (paquete_cpu->operacion) {
            #pragma region PC_A_INSTRUCCION
            case PC_A_INSTRUCCION:
                usleep(retardo_respuesta * 1000);
                // Recibir PID y PC
                t_payload_pc_a_instruccion *payload = (t_payload_pc_a_instruccion *)paquete_cpu->payload;
                int pid = payload->PID;
                int pc = payload->program_counter;
                
                // Obtener instrucción
                char *instruccion = obtenerInstruccion(pid, pc);

                if (instruccion == NULL) {
                    log_info(logger, "Fin de archivo");
                    enviar_paquete_entre(socketCpu, FIN_DE_INSTRUCCIONES, NULL, 0);
                    break;
                }

                // Enviar instrucción a CPU
                t_payload_get_instruccion* payloadGet = malloc(sizeof(t_payload_get_instruccion));
                payloadGet->instruccion = instruccion;

                int size_instruccion;
                void* instruccionSerializada = serializar_get_instruccion(payloadGet, &size_instruccion);
                enviar_paquete_entre(socketCpu, GET_INSTRUCCION, instruccionSerializada, size_instruccion);
                break;
                #pragma endregion PC_A_INSTRUCCION
            
            #pragma region SOLICITAR_DIRECCION_FISICA
            case SOLICITAR_DIRECCION_FISICA:
              t_payload_solicitar_direccion_fisica *payloadSolicitar = paquete_cpu->payload;

              int marco = buscarDireccionFisicaEnTablaDePaginas(payloadSolicitar->PID, payloadSolicitar->pagina);
              
              t_payload_direccion_fisica payloadDireccion = {
                .marco = marco
              };
              enviar_paquete_entre(socketCpu, DIRECCION_FISICA, &payloadDireccion, sizeof(t_payload_direccion_fisica));
              break;
            #pragma endregion SOLICITAR_DIRECCION_FISICA

            #pragma region SOLICITAR_DATO_MEMORIA
            case SOLICITAR_DATO_MEMORIA:
                t_payload_solicitar_dato_memoria *payloadSolicitarDato = paquete_cpu->payload;
                int direccion = payloadSolicitarDato->direccion;
                log_info(logger, "Se llamó a SOLICITAR_DATO_MEMORIA para dirección: %d", direccion);

                // Obtener dato de memoria
                int dato = obtenerDatoMemoria(direccion);
                printf("Dato: %d\n", dato);

                // if (dato == NULL) {
                //     log_info(logger, "No se pudo obtener el dato de memoria");
                //     break;
                // }

                // Enviar dato a CPU
                t_payload_dato_memoria payloadDato = {
                    .dato = (int*)dato
                };
                enviar_paquete_entre(socketCpu, DATO_MEMORIA, &payloadDato, sizeof(t_payload_dato_memoria));

              break;
            #pragma endregion SOLICITAR_DATO_MEMORIA
            
            #pragma region RESIZE_MEMORIA
            case RESIZE_MEMORIA:
                t_payload_resize_memoria *payloadResize = deserializar_resize_memoria(paquete_cpu->payload);
                int pidResize = payloadResize->pid;
                int nuevoTam = payloadResize->tam;
                // log_info(logger, "Se llamó a RESIZE_MEMORIA para nuevo tamaño: %d", nuevoTam);

                int tamActual = obtenerTamanoProceso(pidResize);

                int diff = nuevoTam - tamActual;

                if (diff < 0) {
                  log_info(logger, "Reducción de Proceso: “PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d”", pidResize, tamActual, nuevoTam);
                } else {
                  log_info(logger, "Ampliación de Proceso: “PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d”", pidResize, tamActual, nuevoTam);
                }

                // Redimensionar memoria
                int resultado = redimensionarProceso(pidResize, nuevoTam);

                OP_CODES_ENTRE op_code = resultado < 0 ? ERROR_OUT_OF_MEMORY : RESIZE_SUCCESS;

                // Enviar confirmación a CPU
                enviar_paquete_entre(socketCpu, op_code, paquete_cpu->payload, sizeof(paquete_cpu->payload));
              break;
            #pragma endregion
            #pragma region ENVIAR_DATO_MEMORIA
            case ENVIAR_DATO_MEMORIA:
              t_payload_enviar_dato_memoria *payloadEnviar = deserializar_enviar_dato_memoria(paquete_cpu->payload);
              int direccionEnviar = payloadEnviar->direccion;
              void* datoEnviar = payloadEnviar->dato;
              int tamDatoEnviar = payloadEnviar->tamDato;
              printf("Escribiendo dato %p en dirección %d\n", datoEnviar, direccionEnviar);
              escribirMemoria(direccionEnviar, datoEnviar, tamDatoEnviar);

              t_payload_dato_memoria* payloadDatoEnviar = malloc(sizeof(t_payload_dato_memoria));
              payloadDatoEnviar->dato = datoEnviar;
              enviar_paquete_entre(socketCpu, DATO_MEMORIA, payloadDatoEnviar, sizeof(t_payload_dato_memoria));
              break;
            #pragma endregion
            default:
                log_info(logger, "Operación desconocida de CPU: %d", paquete_cpu->operacion);
                break;
        }

        // eliminar_paquete(paquete_cpu);
    }
    liberarMemoria();
}

void esperar_paquetes_io() {
  sem_wait(&sem_io);    
  log_info(logger,"Esperando paquetes de IO en el socket %d\n", socketIO);
  while (1) {
    t_paquete_entre *paquete_io = recibir_paquete_entre(socketIO);
    OP_CODES_ENTRE op_code = paquete_io->operacion;
    
    if (paquete_io == NULL || paquete_io->payload == NULL) {
        log_error(logger, "No se pudo recibir el paquete de la IO, cerrando hilo");
        break;
    } 

    switch (op_code) {
      case ESCRIBIR_MEMORIA: 
        t_payload_escribir_memoria *payloadStdin = deserializar_escribir_memoria(paquete_io->payload);
        int pid = payloadStdin->direccion;
        char* string = payloadStdin->cadena;
        int sizeString = payloadStdin->size_cadena;

        break;
      default:
        log_info(logger, "Operación desconocida de IO");
        break;
    }
  }
}