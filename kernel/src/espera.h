#ifndef ESPERA_H
#define ESPERA_H

#include <commons/collections/list.h>
#include <utils/constants.h>
#include <utils/client.h>
#include <utils/server.h>
#include "planificacion.h"
#include "global.h"
#include <utils/serializacion.h>
#include "recursos.h"
#include "manejoInterfaces.h"
// void* conectarse_cpu_interrupt(void*);

void atender_cliente(void*);   

void* esperar_paquetes_memoria(void*);
void* esperar_paquetes_cpu_dispatch(void*);

// void esperar_paquetes_cpu_dispatch(int);
void interrumpir();
void enviar_instrucciones_memoria(char*,int);
void enviar_paquete_cpu_dispatch (OP_CODES_ENTRE,void*,int);
void enviar_paquete_memoria(OP_CODES_ENTRE , void*,int);


#endif // ESPERA_H
