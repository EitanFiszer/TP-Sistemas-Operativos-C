#include "conexiones.h"

extern t_log* logger;
extern int socketMemoria;
extern int socketKernel;

void enviar_pcb_kernel(t_PCB *pcb, OP_CODES_ENTRE operacion) {
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete=malloc(sizeof(t_paquete_entre));  

    paquete->operacion = operacion;
    paquete->payload = pcb;
    
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
    enviar_paquete(paq, socketKernel);
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

void* solicitar_dato_memoria(int dirFisica) {
    t_payload_solicitar_dato_memoria* payload = malloc(sizeof(t_payload_solicitar_dato_memoria));
    payload->direccion = dirFisica;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = SOLICITAR_DATO_MEMORIA;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketMemoria);

    t_list* paqueteRecibido = recibir_paquete(socketMemoria);
    if (paqueteRecibido == NULL) {
        return NULL;
    }

    t_paquete_entre* paqueteRecibidoEntero = list_get(paqueteRecibido, 0);
    if (paqueteRecibidoEntero == NULL) {
        return NULL;
    }

    t_payload_dato_memoria* payloadRecibido = paqueteRecibidoEntero->payload;
    void* dato = payloadRecibido->dato;

    return dato;
}

int enviar_dato_memoria(int dirFisica, void* dato) {
    t_payload_enviar_dato_memoria* payload = malloc(sizeof(t_payload_enviar_dato_memoria));
    payload->direccion = dirFisica;
    payload->dato = dato;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = ENVIAR_DATO_MEMORIA;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketMemoria);

    t_list* paqueteRecibido = recibir_paquete(socketMemoria);
    if (paqueteRecibido == NULL) {
        return -1;
    }

    t_paquete_entre* paqueteRecibidoEntero = list_get(paqueteRecibido, 0);
    if (paqueteRecibidoEntero == NULL) {
        return -1;
    }

    t_payload_dato_memoria* payloadRecibido = paqueteRecibidoEntero->payload;
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

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = RESIZE_MEMORIA;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketMemoria);

    t_list* paqueteRecibido = recibir_paquete(socketMemoria);
    if (paqueteRecibido == NULL) {
        return -1;
    }

    t_paquete_entre* paqueteRecibidoEntero = list_get(paqueteRecibido, 0);
    if (paqueteRecibidoEntero == NULL) {
        return -1;
    }

    t_payload_resize_memoria* payloadRecibido = paqueteRecibidoEntero->payload;
    int resultado = payloadRecibido->tam;

    return resultado;
}

void solicitar_wait(char* recurso) {
    t_payload_wait* payload = malloc(sizeof(t_payload_wait));
    payload->recurso = recurso;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = WAIT;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);
}

void solicitar_signal(char* recurso) {
    t_payload_signal* payload = malloc(sizeof(t_payload_signal));
    payload->recurso = recurso;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = SIGNAL;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);
}

char* solicitar_io_stdin(int tam) {
    t_payload_io_stdin_read* payload = malloc(sizeof(t_payload_io_stdin_read));
    payload->tam = tam;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = IO_STDIN_READ;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);

    t_list* paqueteRecibido = recibir_paquete(socketKernel);
    if (paqueteRecibido == NULL) {
        return NULL;
    }

    t_paquete_entre* paqueteRecibidoEntero = list_get(paqueteRecibido, 0);
    if (paqueteRecibidoEntero == NULL) {
        return NULL;
    }

    t_payload_recibir_string_io_stdin* payloadRecibido = paqueteRecibidoEntero->payload;
    char* resultado = payloadRecibido->string;

    return resultado;
}

void solicitar_io_stdout(char* interfaz, char* regDire, char* regTam){
    t_payload_io_stdout_write* payload = malloc(sizeof(t_payload_io_stdout_write));
    payload->interfaz = interfaz;
    payload->regDire = regDire;
    payload->regTam = regTam;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = IO_STDOUT_WRITE;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);
}

void solicitar_fs_createORdelete(char* interfaz, char* nombreArchivo, OP_CODES_ENTRE oper){
    t_payload_fs_create* payload = malloc(sizeof(t_payload_fs_create));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = oper;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);
}

void solicitar_fs_truncate(char* interfaz, char* nombreArchivo, char* regTam){
    t_payload_fs_truncate* payload = malloc(sizeof(t_payload_fs_truncate));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->regTam = regTam;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = IO_FS_TRUNCATE;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);
}

void solicitar_fs_writeORread(char* interfaz, char* nombreArchivo, char* regTam, char* regDire, char* regPuntero, OP_CODES_ENTRE oper){
    t_payload_fs_writeORread* payload = malloc(sizeof(t_payload_fs_writeORread));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->regTam = regTam;
    payload->regDire = regDire;
    payload->regPuntero = regPuntero;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = oper;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);
}


