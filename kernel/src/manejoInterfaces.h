#ifndef MANINTERFAZ_H
#define MANINTERFAZ_H

#include <commons/collections/dictionary.h>
#include <utils/constants.h>
#include "planificacion.h"
#include <utils/envios.h>
#include <utils/serializacion.h>
void inicializar_interfaces();

void agregar_interfaz(char*, char*, int);
void desconectar_IO(char*);
void atender_io_stdin_read(t_payload_io_stdin_read* );
#endif /* MANINTERFAZ_H */