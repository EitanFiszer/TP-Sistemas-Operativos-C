#ifndef CONEXIONES_H
#define CONEXIONES_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include <utils/client.h>
#include <utils/server.h>

void esperar_paquetes_kernel(int socketKernel, t_log* logger, char* path_instrucciones);
void esperar_paquetes_cpu(int socketCpu, t_log* logger);

typedef struct {
    int pid;
    char* path;
} t_payload_crear_proceso;

typedef struct {
    int pid;
} t_payload_finalizar_proceso;

typedef struct {
    int PID;
    int program_counter;
} t_payload_pc_a_instruccion;

#endif
