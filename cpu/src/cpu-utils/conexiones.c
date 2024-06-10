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
    char* dest = "Memoria";

    int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

    if (conexion_memoria < 0) {
		log_error(logger, "No se pudo conectar a %s!", dest);
		return handData;
	} else {
		log_info(logger, "Conectado a %s %s:%s -- %d", dest, ip_memoria, puerto_memoria, conexion_memoria);
	}


    uint32_t modulo = CPU;
    send(conexion_memoria, &modulo, sizeof(uint32_t), 0);
    recv(conexion_memoria, &handData.socket, sizeof(int), MSG_WAITALL);
    recv(conexion_memoria, &handData.tam_pagina, sizeof(int), MSG_WAITALL);

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
