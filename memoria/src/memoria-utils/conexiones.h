#ifndef CONEXIONES_H
#define CONEXIONES_H

#include <commons/log.h>
#include <commons/collections/list.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/constants.h>

void esperar_paquetes_kernel();
void esperar_paquetes_cpu();
void esperar_paquetes_io();
void esperar_clientes_io(void *args);
void atender_cliente_io(void *socket);
void agregar_interfaz(char *nombre, int socket);

#endif