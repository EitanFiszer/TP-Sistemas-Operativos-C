#ifndef CONEXIONES_H
#define CONEXIONES_H

#include <utils/client.h> 
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>

void esperar_paquetes_cpu_dispatch(int);
void esperar_paquetes_memoria(int);
void enviar_instrucciones_memoria(char*,int,int);
void enviar_paquete_cpu_dispatch (OP_CODES_ENTRE,void*, int);
void interrumpir(int);

#endif /* CONEXIONES_H*/