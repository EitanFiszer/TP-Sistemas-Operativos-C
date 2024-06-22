#ifndef SERIALIZACION_H
#define SERIALIZACION_H

#include "constants.h"

// t_payload_dato_memoria* deserializar_dato_memoria(void* buffer);
// void* serializar_dato_memoria(t_payload_dato_memoria* payload, int* size_payload);


// t_payload_enviar_dato_memoria* deserializar_enviar_dato_memoria(void* buffer);
// void* serializar_enviar_dato_memoria(t_payload_enviar_dato_memoria* payload, int* size_payload);



// t_payload_io_stdin_read* deserializar_io_stdin_read(void* buffer);
// void* serializar_io_stdin_read(t_payload_io_stdin_read* payload, int* size_payload);


t_payload_wait_signal* deserializar_wait_signal(void* buffer);
void* serializar_wait_signal(t_payload_wait_signal* payload, int* size_payload);


t_payload_crear_proceso* deserializar_crear_proceso(void* buffer);
void* serializar_crear_proceso(t_payload_crear_proceso* payload, int* size_payload);

t_payload_io_stdout_write* deserializar_io_stdout_write(void* buffer);
void* serializar_io_stdout_write(t_payload_io_stdout_write* payload, int* size_payload);

t_payload_fs_create* deserializar_fs_create(void* buffer);
void* serializar_fs_create(t_payload_fs_create* payload, int* size_payload);

t_payload_recibir_string_io_stdin* deserializar_recibir_string_io_stdin(void* buffer);
void* serializar_recibir_string_io_stdin(t_payload_recibir_string_io_stdin* payload, int* size_payload);


//FALTAN
t_payload_instruccion_io* deserializar_instruccion_io(void* buffer);
void* serializar_instruccion_io(t_payload_instruccion_io* payload, int* size_payload);

t_payload_io_gen_sleep* deserializar_io_gen_sleep(void* buffer);
void* serializar_io_gen_sleep(t_payload_io_gen_sleep* payload, int* size_payload);

t_payload_interfaz_creada* deserializar_interfaz_creada(void* buffer);
void* serializar_interfaz_creada(t_payload_interfaz_creada* payload, int* size_payload);

t_payload_fs_truncate* deserializar_fs_truncate(void* buffer);
void* serializar_fs_truncate(t_payload_fs_truncate* payload, int* size_payload);

t_payload_fs_writeORread* deserializar_fs_writeORread(void* buffer);
void* serializar_fs_writeORread(t_payload_fs_writeORread* payload, int* size_payload);

#endif /* SERIALIZACION_H */