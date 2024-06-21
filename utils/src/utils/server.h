#ifndef SERVER_H
#define SERVER_H

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "constants.h"

typedef struct {
	ID modulo;
	int socket;
} Handshake;

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(char*, t_log*);
Handshake esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif /* SERVER_H */
