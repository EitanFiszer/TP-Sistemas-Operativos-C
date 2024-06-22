#include "server.h"
#include <errno.h>

int iniciar_servidor(char* puerto, t_log* logger)
{
	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	int ret = listen(socket_servidor, SOMAXCONN);
	if (ret == -1)
	{
		log_error(logger, "Error al escuchar las conexiones entrantes");
	}

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

Handshake esperar_cliente(int socket_servidor, t_log* logger)
{
	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);

	uint32_t handshake;
	uint32_t resultOk = 0;
	uint32_t resultError = -1;
	Handshake handshakeCliente;
	recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);

	
	if (socket_cliente == -1) 	{
		log_error(logger, "Error al aceptar un nuevo cliente");
		send(socket_cliente, &resultError, sizeof(uint32_t), 0);
	} else {
		log_info(logger, "Se conecto un cliente!");
		send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
	}

	handshakeCliente.modulo = handshake;
	handshakeCliente.socket = socket_cliente;

	return handshakeCliente;
}

// Handshake esperar_cliente_IO(int socket_servidor, t_log* logger)
// {
// 	// Aceptamos un nuevo cliente
// 	int socket_cliente;
// 	socket_cliente = accept(socket_servidor, NULL, NULL);

// 	uint32_t handshake;
// 	uint32_t resultOk = 0;
// 	uint32_t resultError = -1;
// 	Handshake handshakeCliente;
// 	recv(socket_cliente, &handshake, sizeof(uint32_t), MSG_WAITALL);

	
// 	if (socket_cliente == -1) 	{
// 		log_error(logger, "Error al aceptar un nuevo cliente");
// 		send(socket_cliente, &resultError, sizeof(uint32_t), 0);
// 	} else {
// 		log_info(logger, "Se conecto un cliente!");
// 		send(socket_cliente, &resultOk, sizeof(uint32_t), 0);
// 	}

// 	handshakeCliente.modulo = handshake;
// 	handshakeCliente.socket = socket_cliente;

// 	return handshakeCliente;
// }

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	int recib = recv(socket_cliente, &cod_op, sizeof(int32_t), MSG_WAITALL);

	if(recib > 0) {
		return cod_op;
	} else {
		// print error
		printf("recv: %s (%d)\n", strerror(errno), errno);
		close(socket_cliente);
		return -1;
	}
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	free(buffer);
}

t_list *recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

// t_paquete_entre *deserializar_paquete_entre(void *buffer)
// {
//     t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
//     int desplazamiento = 0;
//     int size_payload = sizeof(t_PCB);

//     memcpy(&(paquete->operacion), buffer + desplazamiento, sizeof(OP_CODES_ENTRE));
//     desplazamiento += sizeof(OP_CODES_ENTRE);

//     paquete->payload = malloc(size_payload);
//     memcpy(paquete->payload, buffer + desplazamiento, size_payload);

//     return paquete;
// }

t_paquete_entre *deserializar_paquete_entre(void *buffer)
{
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    int desplazamiento = 0;

    memcpy(&(paquete->operacion), buffer + desplazamiento, sizeof(OP_CODES_ENTRE));
    desplazamiento += sizeof(OP_CODES_ENTRE);
    memcpy(&(paquete->size_payload), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    paquete->payload = malloc(paquete->size_payload);
    memcpy(paquete->payload, buffer + desplazamiento, paquete->size_payload);

    return paquete;
}

t_paquete_entre* recibir_paquete_entre(int socket_cliente)
{
	printf("Recibiendo paquete entre de %d", socket_cliente);
	int op = recibir_operacion(socket_cliente);

	if (op == -1)
	{
		log_error(logger, "Error al recibir el código de operación");
		return NULL;
	}

	printf("Operacion: %d\n", op);
 
    int size;
    void *buffer = recibir_buffer(&size, socket_cliente);

    int desplazamiento = 0;

    // Leer el tamaño del `t_paquete_entre`
    int size_paquete_entre;
    memcpy(&size_paquete_entre, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    // Deserializar el `t_paquete_entre`
    t_paquete_entre *paquete_entre = deserializar_paquete_entre(buffer + desplazamiento);
    
    free(buffer);
    return paquete_entre;
}

void recibir_paquete_completo(int socket_cliente)
{
    int size;
    void *buffer = recibir_buffer(&size, socket_cliente);

    int desplazamiento = 0;
    while (desplazamiento < size)
    {
        int size_paquete_entre;
        memcpy(&size_paquete_entre, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);

        t_paquete_entre *paquete_entre = deserializar_paquete_entre(buffer + desplazamiento);
        desplazamiento += size_paquete_entre;

        if (paquete_entre->operacion == EXEC_PROCESO)
        {
            t_PCB *pcb = (t_PCB *)paquete_entre->payload;
            // Procesa la PCB
            log_info(logger, "Recibí una PCB con PID: %d y PC: %d", pcb->PID, pcb->program_counter);
        }

        free(paquete_entre->payload);
        free(paquete_entre);
    }
    free(buffer);
}
