#ifndef ESPERA_H
#define ESPERA_H

#include <commons/collections/list.h>
#include <utils/constants.h>
#include <utils/client.h>
#include <utils/server.h>
#include "planificacion.h"

void esperar_paquetes_memoria(int);
void esperar_paquetes_cpu_dispatch(int);
void interrumpir(int);
void enviar_instrucciones_memoria(char*,int,int);
void enviar_paquete_cpu_dispatch (OP_CODES_ENTRE,void*, int);


#endif // ESPERA_H
