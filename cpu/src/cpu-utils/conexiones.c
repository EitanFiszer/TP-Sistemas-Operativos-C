#include "conexiones.h"

extern t_log* logger;

void enviar_pcb_kernel(t_PCB *pcb, int socketKernel, OP_CODES_ENTRE operacion) {
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
		log_error(logger, string_from_format("No se pudo conectar a %s!", dest));
		return handData;
	} else {
		char* mensaje_log = string_from_format("Conectado a %s %s:%s -- %d", dest, ip_memoria, puerto_memoria, conexion_memoria);
		log_info(logger, mensaje_log);
	}


    uint32_t modulo = CPU;
    send(conexion_memoria, &modulo, sizeof(uint32_t), 0);
    recv(conexion_memoria, &handData.socket, sizeof(int), MSG_WAITALL);
    recv(conexion_memoria, &handData.tam_pagina, sizeof(int), MSG_WAITALL);

    return handData;
}