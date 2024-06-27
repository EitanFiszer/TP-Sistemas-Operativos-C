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

#endif
