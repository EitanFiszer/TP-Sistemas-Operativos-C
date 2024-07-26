#include "conexiones.h"
#include <utils/envios.h>

extern t_log* logger;
extern int socketMemoria;
extern int socketKernel;

void enviar_pcb_kernel(t_PCB *pcb, OP_CODES_ENTRE operacion) {
    // printf("Enviando PCB al kernel, PID: %d\n", pcb->PID);
    enviar_paquete_entre(socketKernel, operacion, pcb, sizeof(t_PCB));
    sleep(1);
}

handshake_cpu_memoria handshake_memoria(char* ip_memoria, char* puerto_memoria) {
    handshake_cpu_memoria handData = { .socket = -1, .tam_pagina = -1 };
    
    handData.socket = crear_conexion(ip_memoria, puerto_memoria);

    if (handData.socket == -1) {
        log_error(logger, "No se pudo conectar con la memoria");
        return handData;
    } else {
        log_info(logger, "Conexión establecida con la memoria");
    }

    uint32_t modulo = CPU;

    send(handData.socket, &modulo, sizeof(uint32_t), 0);
    recv(handData.socket, &handData.tam_pagina, sizeof(uint32_t), 0);

    return handData;
}

void* solicitar_dato_memoria(int dirFisica, int tam) {
    t_payload_solicitar_dato_memoria* payload = malloc(sizeof(t_payload_solicitar_dato_memoria));
    payload->direccion = dirFisica;
    payload->tam = tam;

    int size_payload;
    void* buffer = serializar_solicitar_dato_memoria(payload, &size_payload); 
    enviar_paquete_entre(socketMemoria, SOLICITAR_DATO_MEMORIA, buffer, size_payload);   

    t_paquete_entre* paqueteRecibidoEntero = recibir_paquete_entre(socketMemoria);
    if (paqueteRecibidoEntero == NULL) {
        return NULL;
    }
    t_payload_dato_memoria* payloadRecibido = paqueteRecibidoEntero->payload;
    void* dato = payloadRecibido->dato;

    return dato;
}

int enviar_dato_memoria(int dirFisica, void* dato, int tamDato) {
    t_payload_enviar_dato_memoria* payload = malloc(sizeof(t_payload_enviar_dato_memoria));
    payload->direccion = dirFisica;
    payload->dato = dato;
    payload->tamDato = tamDato;

    int size_payload;
    void* buffer = serializar_enviar_dato_memoria(payload, &size_payload);

    // printf("Enviando dato void %p a direccion %d", dato, dirFisica);
    
    enviar_paquete_entre(socketMemoria, ENVIAR_DATO_MEMORIA, buffer, size_payload);

    t_paquete_entre* paqueteRecibido = recibir_paquete_entre(socketMemoria);
    if (paqueteRecibido == NULL) {
        return -1;
    }

    t_payload_dato_memoria* payloadRecibido = (t_payload_dato_memoria*)paqueteRecibido->payload;
    void* resultado = payloadRecibido->dato;

    if (resultado == NULL) {
        return -1;
    }

    return *(int*)resultado;
}

int solicitar_resize_memoria(int pid, int tam) {
    t_payload_resize_memoria* payload = malloc(sizeof(t_payload_resize_memoria));
    payload->pid = pid;
    payload->tam = tam;

    int size_payload;
    void *buffer = serializar_resize_memoria(payload, &size_payload);

    enviar_paquete_entre(socketMemoria, RESIZE_MEMORIA, buffer, sizeof(t_payload_resize_memoria));
    
    t_paquete_entre* paqueteRecibidoEntero = recibir_paquete_entre(socketMemoria);
    if (paqueteRecibidoEntero == NULL || paqueteRecibidoEntero->operacion != RESIZE_SUCCESS) {
        return -1;
    }

    t_payload_resultado_resize_memoria* payloadRecibido = deserializar_resultado_resize_memoria(paqueteRecibidoEntero->payload);
    int resultado = payloadRecibido->resultado;

    return resultado == ERROR_OUT_OF_MEMORY ? -1 : resultado;
}

void solicitar_wait(char* recurso, t_PCB* pcb) {
    t_payload_wait_signal* payload = malloc(sizeof(t_payload_wait_signal));
    payload->recurso = recurso;
    payload->pcb = pcb;

    printf("Solicitando wait de recurso %s en el PID %d\n", recurso, pcb->PID);

    int size_payload;
    void* buffer = serializar_wait_signal(payload,&size_payload);
    enviar_paquete_entre(socketKernel, WAIT, buffer, size_payload);
}

void solicitar_signal(char* recurso, t_PCB* pcb) {
    t_payload_wait_signal* payload = malloc(sizeof(t_payload_wait_signal));
    payload->recurso = recurso;
    payload->pcb = pcb;

    printf("Solicitando signal de recurso %s en el PID %d\n", recurso, pcb->PID);
    
    int size_payload;
    void* buffer = serializar_wait_signal(payload,&size_payload);
    enviar_paquete_entre(socketKernel, SIGNAL, buffer, size_payload);
}

void solicitar_io_stdin(int tam, t_PCB* pcb, char* interfaz, char* regTam, int dirFisica) {
    t_payload_io_stdin_read* payload = malloc(sizeof(t_payload_io_stdin_read));
    payload->tam = tam;
    payload->pcb = pcb;
    payload->interfaz = interfaz;
    payload->dirFisica = dirFisica;

    int size_payload;
    void* buffer = serializar_io_stdin_read(payload, &size_payload);
    enviar_paquete_entre(socketKernel, IO_STDIN_READ, buffer, size_payload);
}

void solicitar_io_stdout(char* interfaz, int direccionFisica, int tam, t_PCB* pcb){
    t_payload_io_stdout_write* payload = malloc(sizeof(t_payload_io_stdout_write));
    payload->interfaz = interfaz;
    payload->direccionFisica = direccionFisica;
    payload->tam = tam;
    payload->pcb = pcb;
    int size_payload;
    void* buffer =serializar_io_stdout_write(payload,&size_payload);

   enviar_paquete_entre(socketKernel, IO_STDOUT_WRITE, buffer, size_payload);
}

void solicitar_fs_createORdelete(char* interfaz, char* nombreArchivo, OP_CODES_ENTRE oper, t_PCB* pcb){
    t_payload_fs_create* payload = malloc(sizeof(t_payload_fs_create));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->pcb = pcb;
    int size_payload;
    void* buffer = serializar_fs_create(payload,&size_payload);
    enviar_paquete_entre(socketKernel, oper, buffer, size_payload);
}

void solicitar_fs_truncate(char* interfaz, char* nombreArchivo, char* regTam, t_PCB* pcb){
    t_payload_fs_truncate* payload = malloc(sizeof(t_payload_fs_truncate));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->regTam = regTam;
    payload->pcb = pcb;
    int size_payload;
    void* buffer = serializar_fs_truncate(payload, &size_payload);
    enviar_paquete_entre(socketKernel, IO_FS_TRUNCATE, buffer, size_payload);
}

void solicitar_fs_writeORread(char* interfaz, char* nombreArchivo, char* regTam, char* regDire, char* regPuntero, OP_CODES_ENTRE oper, t_PCB* pcb){
    t_payload_fs_writeORread* payload = malloc(sizeof(t_payload_fs_writeORread));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->regTam = regTam;
    payload->regDire = regDire;
    payload->regPuntero = regPuntero;
    payload->pcb = pcb;

    int size_payload;
    void* buffer = serializar_fs_writeORread(payload, &size_payload);

    enviar_paquete_entre(socketKernel, oper, buffer, size_payload);
}


