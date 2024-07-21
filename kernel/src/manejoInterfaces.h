#ifndef MANINTERFAZ_H
#define MANINTERFAZ_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/dictionary.h>
#include <utils/constants.h>
#include "planificacion.h"
#include <utils/envios.h>
#include <utils/serializacion.h>
void inicializar_interfaces();
void inicializar_operaciones_en_espera();
char* int_to_string(int );
void agregar_interfaz(char*, char*, int);



void removerBuffer(void* );
void desconectar_IO(char*);
void desocupar_io(char*);

void atender_io_stdin_read(t_payload_io_stdin_read* );
void atender_io_stdout_write(t_payload_io_stdout_write*);
void atender_fs_createOrDelate(t_payload_fs_create*, OP_CODES_ENTRE);
void atender_fs_truncate(t_payload_fs_truncate*);
void atender_fs_writeOrRead(t_payload_fs_writeORread*, OP_CODES_ENTRE);
void atender_io_gen_sleep(t_payload_io_gen_sleep*);



#endif /* MANINTERFAZ_H */