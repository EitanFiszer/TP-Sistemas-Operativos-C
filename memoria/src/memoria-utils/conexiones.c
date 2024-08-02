#include "./conexiones.h"
#include <commons/log.h>
#include <commons/memory.h>
#include <utils/serializacion.h>
#include <utils/envios.h>
#include <memoria-utils/procesos.h>
#include <memoria-utils/memoria.h>
#include <semaphore.h>

extern t_log *logger;
extern char *path_instrucciones;
extern int retardo_respuesta;
extern int socketKernel;
extern int socketCpu;
extern int socketIO;
extern int server_fd;
extern int TAM_MEMORIA;
extern Memoria memoria;

t_dictionary *interfaces_dict;

void esperar_paquetes_kernel() {
    printf("Esperando paquetes de KERNEL en el socket %d\n", socketKernel);

    while (1) {
        t_paquete_entre *paquete = recibir_paquete_entre(socketKernel);
        if (paquete == NULL) {
            log_error(logger, "No se pudo recibir el paquete del KERNEL, cerrando hilo");
            liberarMemoria();
            break;
        }

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
                    // Escribir toda la memoria
                    // mem_hexdump(memoria.memoria, TAM_MEMORIA);
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
              usleep(retardo_respuesta * 1000);
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
                usleep(retardo_respuesta * 1000);
                t_payload_solicitar_dato_memoria *payloadSolicitarDato = deserializar_solicitar_dato_memoria(paquete_cpu->payload);
                int direccion = payloadSolicitarDato->direccion;
                log_info(logger, "Se llamó a SOLICITAR_DATO_MEMORIA para dirección: %d", direccion);

                int tamDato = payloadSolicitarDato->tam;
                int pidDatoMemoria = payloadSolicitarDato->pid; 
                // Obtener dato de memoria
                void* dato = obtenerDatoMemoria(pidDatoMemoria, direccion, tamDato);
                
                enviar_paquete_entre(socketCpu, DATO_MEMORIA, dato, sizeof(dato));
              break;
            #pragma endregion SOLICITAR_DATO_MEMORIA
            
            #pragma region RESIZE_MEMORIA
            case RESIZE_MEMORIA:
                usleep(retardo_respuesta * 1000);
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

                t_payload_resultado_resize_memoria* payloadRes = malloc(sizeof(t_payload_resultado_resize_memoria));
                payloadRes->resultado = resultado;

                int size_payload;
                void* buffer = serializar_resultado_resize_memoria(payload, &size_payload);

                // Enviar confirmación a CPU
                enviar_paquete_entre(socketCpu, op_code, buffer, size_payload);
              break;
            #pragma endregion
            #pragma region ESCRIBIR_MEMORIA
            case ESCRIBIR_MEMORIA:
              usleep(retardo_respuesta * 1000);
              t_payload_escribir_memoria* payloadEscribir = deserializar_escribir_memoria(paquete_cpu->payload);
              int direccionEnviar = payloadEscribir->direccion;
              void* datoEnviar = payloadEscribir->dato;
              int tamDatoEnviar = payloadEscribir->size_cadena;
              int pidEscribir = payloadEscribir->pid;
              // printf("Escribiendo dato %p en dirección %d\n", datoEnviar, direccionEnviar);
              escribirMemoria(pidEscribir, direccionEnviar, datoEnviar, tamDatoEnviar);
              
              enviar_paquete_entre(socketCpu, DATO_MEMORIA, datoEnviar, sizeof(datoEnviar));
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


void atender_cliente_io(void *socket) {
    char *nombre_io_hilo = NULL;
    int socket_cliente_IO = *(int *)socket;
    interfaces_dict = dictionary_create();
    free(socket);
    // bool salir = true;
    log_info(logger, "Atendiendo cliente IO en el socket %d", socket_cliente_IO);
    while (1) {
        if (socket_cliente_IO == -1) {
            log_error(logger, "Cliente IO desconectado, socket: %d", socket_cliente_IO);
            if (nombre_io_hilo != NULL) {
                desconectar_IO(nombre_io_hilo);
            }
            break;
        }
        t_paquete_entre *paqueteEntre = recibir_paquete_entre(socket_cliente_IO);
        if (paqueteEntre == NULL) {
          log_error(logger, "Cliente IO desconectado, socket: %d  nombre %s", socket_cliente_IO, nombre_io_hilo);
          if (nombre_io_hilo != NULL) {
              desconectar_IO(nombre_io_hilo);
          }
          break;
        }

        log_info(logger, "Operación de IO socket %d: %d", socket_cliente_IO, paqueteEntre->operacion);

        switch (paqueteEntre->operacion) {
          case IO_INTERFAZ_CREADA: 
            t_payload_interfaz_creada *datos_interfaz = deserializar_interfaz_creada(paqueteEntre->payload);
            nombre_io_hilo = malloc(strlen(datos_interfaz->nombre) + 1);
            strcpy(nombre_io_hilo, datos_interfaz->nombre);
            agregar_interfaz(datos_interfaz->nombre, socket_cliente_IO);
            log_info(logger, "NUEVA INTERFAZ %s CONECTADA", datos_interfaz->nombre);
            break;
          case SOLICITAR_DIRECCION_FISICA:
              usleep(retardo_respuesta * 1000);
              t_payload_solicitar_direccion_fisica *payloadSolicitar = paqueteEntre->payload;

              int marco = buscarDireccionFisicaEnTablaDePaginas(payloadSolicitar->PID, payloadSolicitar->pagina);
              
              t_payload_direccion_fisica payloadDireccion = {
                .marco = marco
              };

              enviar_paquete_entre(socket_cliente_IO, DIRECCION_FISICA, &payloadDireccion, sizeof(t_payload_direccion_fisica));
              break;
          case ESCRIBIR_MEMORIA:
            usleep(retardo_respuesta * 1000);
              t_payload_escribir_memoria* payloadEscribir = deserializar_escribir_memoria(paqueteEntre->payload);
              int direccionEnviar = payloadEscribir->direccion;
              void* datoEnviar = payloadEscribir->dato;
              int tamDatoEnviar = payloadEscribir->size_cadena;
              int pid = payloadEscribir->pid;
              // printf("Escribiendo dato %p en dirección %d\n", datoEnviar, direccionEnviar);
              escribirMemoria(pid, direccionEnviar, datoEnviar, tamDatoEnviar);
            break;
          case SOLICITAR_DATO_MEMORIA:
            usleep(retardo_respuesta * 1000);
            t_payload_solicitar_dato_memoria *payloadSolicitarDato = deserializar_solicitar_dato_memoria(paqueteEntre->payload);
            int direccion = payloadSolicitarDato->direccion;
            log_info(logger, "Se llamó a SOLICITAR_DATO_MEMORIA para dirección: %d", direccion);

            int tamDato = payloadSolicitarDato->tam;
            int pidDatoMemoria = payloadSolicitarDato->pid; 
            // Obtener dato de memoria
            void* dato = obtenerDatoMemoria(pid, direccion, tamDato);
            
            enviar_paquete_entre(socket_cliente_IO, DATO_MEMORIA, dato,tamDato);
          break;

        }        
    }
}