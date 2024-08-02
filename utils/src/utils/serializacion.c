#include "serializacion.h"
#include <string.h>
#include <stdio.h>

void* serializar_resize_memoria(t_payload_resize_memoria* payload, int* size_payload) {
    *size_payload = sizeof(int) * 2;
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &(payload->pid), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, &(payload->tam), sizeof(int));
    return buffer;
}

t_payload_resize_memoria* deserializar_resize_memoria(void* buffer) {
    t_payload_resize_memoria* payload = malloc(sizeof(t_payload_resize_memoria));
    int desplazamiento = 0;
    memcpy(&(payload->pid), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&(payload->tam), buffer + desplazamiento, sizeof(int));
    return payload;
}

/*
void* serializar_stdin_read_de_kernel_a_io(t_payload_io_stdin_read_de_kernel_a_io* payload, int* size_payload) {
    
    typedef struct {
      char* interfaz;
      int direccionFisica;
	    int tam;
    } t_payload_io_stdin_read_de_kernel_a_io;
    
    int size_interfaz = strlen(payload->interfaz) + 1;
    *size_payload = sizeof(int) + size_interfaz + sizeof(int) * 2;
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &size_interfaz, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->interfaz, size_interfaz);
    desplazamiento += size_interfaz;
    memcpy(buffer + desplazamiento, &payload->direccionFisica, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, &payload->tam, sizeof(int));
    return buffer;
}
*/

/*
t_payload_io_stdin_read_de_kernel_a_io* deserializar_io_stdin_read_de_kernel_a_io(void* buffer) {
    t_payload_io_stdin_read_de_kernel_a_io* payload = malloc(sizeof(t_payload_io_stdin_read_de_kernel_a_io));
    int desplazamiento = 0;
    int size_interfaz;
    memcpy(&size_interfaz, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + desplazamiento, size_interfaz);
    desplazamiento += size_interfaz;
    memcpy(&payload->direccionFisica, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&payload->tam, buffer + desplazamiento, sizeof(int));
    return payload;
}
*/

void* serializar_io_stdin_read(t_payload_io_stdin_read* payload, int* size_payload) {
    /*
    typedef struct {
    int tam;
    t_PCB* pcb;
  	char* interfaz;
  	int dirFisica;
 t_payload_io_stdin_read;
    */

    int size_pcb = sizeof(t_PCB);
    int size_interfaz = strlen(payload->interfaz) + 1;
    *size_payload = sizeof(int) + size_pcb + size_interfaz + sizeof(int);
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, payload->pcb, size_pcb);
    desplazamiento += size_pcb;
    memcpy(buffer + desplazamiento, &payload->tam, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->interfaz, size_interfaz);
    desplazamiento += size_interfaz;
    memcpy(buffer + desplazamiento, &payload->dirFisica, sizeof(int));
    return buffer;
}


t_payload_io_stdin_read* deserializar_io_stdin_read(void* buffer) {
    t_payload_io_stdin_read* payload = malloc(sizeof(t_payload_io_stdin_read));
    int desplazamiento = 0;
    int size_pcb = sizeof(t_PCB);
    payload->pcb = malloc(size_pcb);
    memcpy(payload->pcb, buffer + desplazamiento, size_pcb);
    desplazamiento += size_pcb;
    memcpy(&(payload->tam), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    int size_interfaz = strlen(buffer + desplazamiento) + 1;
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + desplazamiento, size_interfaz);
    desplazamiento += size_interfaz;
    memcpy(&(payload->dirFisica), buffer + desplazamiento, sizeof(int));
    return payload;
}


void* serializar_pcb(t_PCB* pcb, int* size_pcb) {
    *size_pcb = sizeof(t_PCB);
    void* buffer = malloc(*size_pcb);
    memcpy(buffer, pcb, *size_pcb);
    return buffer;
}

t_PCB* deserializar_pcb(void* buffer) {
    t_PCB* pcb = malloc(sizeof(t_PCB));
    memcpy(pcb, buffer, sizeof(t_PCB));
    return pcb;
}

void* serializar_get_instruccion(t_payload_get_instruccion* payload, int *size_payload) {
    int size_instruccion = strlen(payload->instruccion) + 1; // Incluye el carácter nulo
    *size_payload = size_instruccion;
    void* buffer = malloc(size_instruccion);

    memcpy(buffer, payload->instruccion, size_instruccion);

    return buffer;
}

t_payload_get_instruccion *deserializar_get_instruccion(void *buffer)
{
    t_payload_get_instruccion *payload = malloc(sizeof(t_payload_get_instruccion));
    payload->instruccion = malloc(strlen(buffer) + 1);
    strcpy(payload->instruccion, buffer);
    return payload;
}

void *serializar_wait_signal(t_payload_wait_signal *payload, int *size_payload)
{
    int size_pcb = sizeof(t_PCB);
    int size_recurso = strlen(payload->recurso) + 1; // Incluye el carácter nulo
    *size_payload = size_pcb + size_recurso;
    void *buffer = malloc(*size_payload);

    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, payload->pcb, size_pcb);
    desplazamiento += size_pcb;
    memcpy(buffer + desplazamiento, payload->recurso, size_recurso);

    return buffer;
}

void *serializar_recibir_string_io_stdin(t_payload_recibir_string_io_stdin *payload, int *size_payload)
{
    int size_string = strlen(payload->string) + 1; // Incluye el carácter nulo
    *size_payload = sizeof(int) + size_string;
    void *buffer = malloc(*size_payload);

    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &size_string, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->string, size_string);

    return buffer;
}
t_payload_recibir_string_io_stdin *deserializar_recibir_string_io_stdin(void *buffer)
{
    t_payload_recibir_string_io_stdin *payload = malloc(sizeof(t_payload_recibir_string_io_stdin));

    int desplazamiento = 0;
    int size_string;
    memcpy(&size_string, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    payload->string = malloc(size_string);
    memcpy(payload->string, buffer + desplazamiento, size_string);

    return payload;
}
void *serializar_crear_proceso(t_payload_crear_proceso *payload, int *size_payload)
{
    int size_path = strlen(payload->path) + 1; // Incluye el carácter nulo
    *size_payload = sizeof(int) + size_path;
    void *buffer = malloc(*size_payload);

    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &(payload->pid), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->path, size_path);

    return buffer;
}
void *serializar_fs_create(t_payload_fs_create *payload, int *size_payload)
{
    int size_interfaz = strlen(payload->interfaz) + 1;           // Incluye el carácter nulo
    int size_nombreArchivo = strlen(payload->nombreArchivo) + 1; // Incluye el carácter nulo

    *size_payload = 2 * sizeof(int) + size_interfaz + size_nombreArchivo;
    void *buffer = malloc(*size_payload);

    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &size_interfaz, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->interfaz, size_interfaz);
    desplazamiento += size_interfaz;

    memcpy(buffer + desplazamiento, &size_nombreArchivo, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->nombreArchivo, size_nombreArchivo);

    return buffer;
}

// t_payload_dato_memoria* deserializar_dato_memoria(void* buffer) {
//     t_payload_dato_memoria* payload = malloc(sizeof(t_payload_dato_memoria));
//     int desplazamiento = 0;

//     memcpy(&(payload->size_dato), buffer + desplazamiento, sizeof(int));
//     desplazamiento += sizeof(int);

//     payload->dato = malloc(payload->size_dato);
//     memcpy(payload->dato, buffer + desplazamiento, payload->size_dato);

//     return payload;
// }
// t_payload_enviar_dato_memoria* deserializar_enviar_dato_memoria(void* buffer) {
//     t_payload_enviar_dato_memoria* payload = malloc(sizeof(t_payload_enviar_dato_memoria));
//     int desplazamiento = 0;

//     memcpy(&(payload->direccion), buffer + desplazamiento, sizeof(int));
//     desplazamiento += sizeof(int);
//     memcpy(&(payload->size_dato), buffer + desplazamiento, sizeof(int));
//     desplazamiento += sizeof(int);

//     payload->dato = malloc(payload->size_dato);
//     memcpy(payload->dato, buffer + desplazamiento, payload->size_dato);

//     return payload;
// }

// t_payload_io_stdin_read* deserializar_io_stdin_read(void* buffer) {
//     t_payload_io_stdin_read* payload = malloc(sizeof(t_payload_io_stdin_read));
//     // Suponiendo que la estructura t_PCB tiene un tamaño fijo conocido como SIZE_T_PCB
//     int size_pcb = sizeof(t_PCB);

//     payload->pcb = malloc(size_pcb);
//     memcpy(payload->pcb, buffer, size_pcb);
//     memcpy(&(payload->tam), buffer + size_pcb, sizeof(int));

//     return payload;
// }

t_payload_wait_signal *deserializar_wait_signal(void *buffer)
{
  /*
  typedef struct {
    t_PCB* pcb;
    char* recurso;
  } t_payload_wait_signal;
  */
    t_payload_wait_signal *payload = malloc(sizeof(t_payload_wait_signal));
    int desplazamiento = 0;
    int size_pcb = sizeof(t_PCB);
    payload->pcb = malloc(size_pcb);
    memcpy(payload->pcb, buffer + desplazamiento, size_pcb);
    desplazamiento += size_pcb;
    payload->recurso = strdup(buffer + desplazamiento);
    return payload;
}
t_payload_crear_proceso *deserializar_crear_proceso(void *buffer)
{
    t_payload_crear_proceso *payload = malloc(sizeof(t_payload_crear_proceso));

    int desplazamiento = 0;
    memcpy(&(payload->pid), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    payload->path = strdup(buffer + desplazamiento);

    return payload;
}
void *serializar_io_stdout_write(t_payload_io_stdout_write *payload, int *size_payload){
    /*
      typedef struct{
        char* interfaz;
        int direccionFisica;
        int tam;
        t_PCB* pcb;
      }t_payload_io_stdout_write;
    */
    int size_interfaz = strlen(payload->interfaz) + 1;
    int size_pcb = sizeof(t_PCB);
    *size_payload = sizeof(int) + size_interfaz + sizeof(int) * 2 + size_pcb;
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &size_interfaz, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->interfaz, size_interfaz);
    desplazamiento += size_interfaz;
    memcpy(buffer + desplazamiento, &payload->direccionFisica, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, &payload->tam, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->pcb, size_pcb);
    return buffer;

}

t_payload_io_stdout_write *deserializar_io_stdout_write(void *buffer){
    t_payload_io_stdout_write *payload = malloc(sizeof(t_payload_io_stdout_write));
    int desplazamiento = 0;
    int size_interfaz;
    memcpy(&size_interfaz, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + desplazamiento, size_interfaz);
    desplazamiento += size_interfaz;
    memcpy(&payload->direccionFisica, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&payload->tam, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    int size_pcb = sizeof(t_PCB);
    payload->pcb = malloc(size_pcb);
    memcpy(payload->pcb, buffer + desplazamiento, size_pcb);
    return payload;
}

t_payload_fs_create *deserializar_fs_create(void *buffer){
    t_payload_fs_create *payload = malloc(sizeof(t_payload_fs_create));
    int offset = 0;

    int size_interfaz;
    memcpy(&size_interfaz, buffer + offset, sizeof(int));
    offset += sizeof(int);
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + offset, size_interfaz);
    offset += size_interfaz;

    int size_nombreArchivo;
    memcpy(&size_nombreArchivo, buffer + offset, sizeof(int));
    offset += sizeof(int);
    payload->nombreArchivo = malloc(size_nombreArchivo);
    memcpy(payload->nombreArchivo, buffer + offset, size_nombreArchivo);

    return payload;
}
t_payload_instruccion_io *deserializar_instruccion_io(void *buffer)
{
    t_payload_instruccion_io *payload = malloc(sizeof(t_payload_instruccion_io));
    int offset = 0;

    memcpy(&payload->interfaz, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int size_op = strlen(buffer + offset) + 1;
    payload->op = malloc(size_op);
    memcpy(payload->op, buffer + offset, size_op);
    offset += size_op;

    memcpy(&payload->tiempo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    return payload;
}
void *serializar_instruccion_io(t_payload_instruccion_io *payload, int *size_payload)
{
    int size_op = strlen(payload->op) + 1; // Incluye el carácter nulo
    *size_payload = sizeof(int) + size_op + sizeof(int);

    void *buffer = malloc(*size_payload);
    int offset = 0;

    memcpy(buffer + offset, &payload->interfaz, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, payload->op, size_op);
    offset += size_op;

    memcpy(buffer + offset, &payload->tiempo, sizeof(int));
    offset += sizeof(int);

    return buffer;
}

t_payload_io_gen_sleep *deserializar_io_gen_sleep(void *buffer){
    /*
    typedef struct {
      char* interfaz;
      int tiempo;
      t_PCB* pcb;
    } t_payload_io_gen_sleep;
    */

    t_payload_io_gen_sleep *payload = malloc(sizeof(t_payload_io_gen_sleep));
    int offset = 0;

    int size_interfaz = strlen(buffer + offset) + 1;
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + offset, size_interfaz);
    offset += size_interfaz;

    memcpy(&payload->tiempo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int size_pcb = sizeof(t_PCB);
    payload->pcb = malloc(size_pcb);
    memcpy(payload->pcb, buffer + offset, size_pcb);

    return payload;
}

void *serializar_io_gen_sleep(t_payload_io_gen_sleep *payload, int *size_payload){
    /*
    typedef struct {
      char* interfaz;
      int tiempo;
      t_PCB* pcb;
    } t_payload_io_gen_sleep;
    */
    int size_interfaz = strlen(payload->interfaz) + 1;
    int size_pcb = sizeof(t_PCB);
    *size_payload = size_interfaz + sizeof(int) + size_pcb;
    
    void* buffer = malloc(*size_payload);
    int offset = 0;

    memcpy(buffer + offset, payload->interfaz, size_interfaz);
    offset += size_interfaz;

    memcpy(buffer + offset, &payload->tiempo, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, payload->pcb, size_pcb);
    offset += size_pcb;

    return buffer;
}

t_payload_interfaz_creada *deserializar_interfaz_creada(void *buffer){
    t_payload_interfaz_creada* payload = malloc(sizeof(t_payload_interfaz_creada));
    int offset = 0;

    int size_nombre = strlen(buffer + offset) + 1;
    payload->nombre = malloc(size_nombre);
    memcpy(payload->nombre, buffer + offset, size_nombre);
    offset += size_nombre;

    int size_tipo_interfaz = strlen(buffer + offset) + 1;
    payload->tipo_interfaz = malloc(size_tipo_interfaz);
    memcpy(payload->tipo_interfaz, buffer + offset, size_tipo_interfaz);
    offset += size_tipo_interfaz;

    return payload;
}

void *serializar_interfaz_creada(t_payload_interfaz_creada *payload, int *size_payload){
    int size_nombre = strlen(payload->nombre) + 1;
    int size_tipo_interfaz = strlen(payload->tipo_interfaz) + 1;

    *size_payload = size_nombre + size_tipo_interfaz;

    void* buffer = malloc(*size_payload);
    int offset = 0;

    memcpy(buffer + offset, payload->nombre, size_nombre);
    offset += size_nombre;

    memcpy(buffer + offset, payload->tipo_interfaz, size_tipo_interfaz);
    offset += size_tipo_interfaz;

    return buffer;
}

t_payload_fs_truncate *deserializar_fs_truncate(void *buffer){
    t_payload_fs_truncate* payload = malloc(sizeof(t_payload_fs_truncate));
    int offset = 0;

    int size_interfaz = strlen(buffer + offset) + 1;
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + offset, size_interfaz);
    offset += size_interfaz;

    int size_nombreArchivo = strlen(buffer + offset) + 1;
    payload->nombreArchivo = malloc(size_nombreArchivo);
    memcpy(payload->nombreArchivo, buffer + offset, size_nombreArchivo);
    offset += size_nombreArchivo;

    memcpy(payload->tam, buffer + offset, sizeof(int));
    offset += sizeof(int);

    return payload;
}
void *serializar_fs_truncate(t_payload_fs_truncate *payload, int *size_payload){
    int size_interfaz = strlen(payload->interfaz) + 1;
    int size_nombreArchivo = strlen(payload->nombreArchivo) + 1;

    *size_payload = size_interfaz + size_nombreArchivo + sizeof(int);

    void* buffer = malloc(*size_payload);
    int offset = 0;

    memcpy(buffer + offset, payload->interfaz, size_interfaz);
    offset += size_interfaz;

    memcpy(buffer + offset, payload->nombreArchivo, size_nombreArchivo);
    offset += size_nombreArchivo;

    memcpy(buffer + offset, payload->tam, sizeof(int));
    offset += sizeof(int);

    return buffer;
}

t_payload_fs_writeORread *deserializar_fs_writeORread(void *buffer){
    t_payload_fs_writeORread* payload = malloc(sizeof(t_payload_fs_writeORread));
    int offset = 0;

    int size_interfaz = strlen(buffer + offset) + 1;
    payload->interfaz = malloc(size_interfaz);
    memcpy(payload->interfaz, buffer + offset, size_interfaz);
    offset += size_interfaz;

    int size_nombreArchivo = strlen(buffer + offset) + 1;
    payload->nombreArchivo = malloc(size_nombreArchivo);
    memcpy(payload->nombreArchivo, buffer + offset, size_nombreArchivo);
    offset += size_nombreArchivo;

    memcpy(payload->tam, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(payload->dirFisica, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(payload->punteroArchivo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    int size_pcb = sizeof(t_PCB);
    payload->pcb = malloc(size_pcb);
    memcpy(payload->pcb, buffer + offset, size_pcb);

    return payload;
}
void *serializar_fs_writeORread(t_payload_fs_writeORread *payload, int *size_payload){
    int size_interfaz = strlen(payload->interfaz) + 1;
    int size_nombreArchivo = strlen(payload->nombreArchivo) + 1;
    int size_pcb = sizeof(t_PCB);

    *size_payload = size_interfaz + size_nombreArchivo + sizeof(int) * 3 + size_pcb;

    void* buffer = malloc(*size_payload);
    int offset = 0;

    memcpy(buffer + offset, payload->interfaz, size_interfaz);
    offset += size_interfaz;

    memcpy(buffer + offset, payload->nombreArchivo, size_nombreArchivo);
    offset += size_nombreArchivo;

    memcpy(buffer + offset, payload->tam, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, payload->dirFisica, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, payload->punteroArchivo, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, payload->pcb, size_pcb);
    offset += size_pcb;

    return buffer;
}

void* serializar_escribir_memoria(t_payload_escribir_memoria* payload, int* size_payload) {
    /*
    typedef struct {
      int direccion;
      char* cadena;
      int size_cadena;
      int pid;
    } t_payload_escribir_memoria;
    */

    int size_cadena = payload->size_cadena;
    *size_payload = sizeof(int) * 3 + size_cadena;
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;

    memcpy(buffer + desplazamiento, &(payload->pid), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, &(payload->direccion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, &size_cadena, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(buffer + desplazamiento, payload->dato, size_cadena);
    return buffer;
}

t_payload_escribir_memoria* deserializar_escribir_memoria(void* buffer) {
    t_payload_escribir_memoria* payload = malloc(sizeof(t_payload_escribir_memoria));
    int desplazamiento = 0;

    memcpy(&(payload->pid), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&(payload->direccion), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&(payload->size_cadena), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    payload->dato = malloc(payload->size_cadena);
    memcpy(payload->dato, buffer + desplazamiento, payload->size_cadena);
    return payload;
}

void* serializar_leer_memoria(t_payload_leer_memoria* payload, int* size_payload) {
    *size_payload = sizeof(int) * 2;
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    
    memcpy(buffer + desplazamiento, &(payload->direccion), sizeof(int));
    desplazamiento += sizeof(int);
    
    memcpy(buffer + desplazamiento, &(payload->size_cadena), sizeof(int));
    desplazamiento += sizeof(int);
    
    return buffer;
}

t_payload_leer_memoria* deserializar_leer_memoria(void* buffer) {
    t_payload_leer_memoria* payload = malloc(sizeof(t_payload_leer_memoria));
    int desplazamiento = 0;

    memcpy(&(payload->direccion), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    memcpy(&(payload->size_cadena), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);

    return payload;
}

void* serializar_enviar_dato_memoria(t_payload_enviar_dato_memoria* payload, int* size_payload) {
    *size_payload = sizeof(payload->direccion) + sizeof(payload->tamDato) + payload->tamDato;
    
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    
    if (buffer == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }

    memcpy(buffer + desplazamiento, &(payload->direccion), sizeof(payload->direccion));
    desplazamiento += sizeof(payload->direccion);
    memcpy(buffer + desplazamiento, &(payload->tamDato), sizeof(payload->tamDato));
    desplazamiento += sizeof(payload->tamDato);
    memcpy(buffer + desplazamiento, payload->dato, payload->tamDato);

    return buffer;
}

t_payload_enviar_dato_memoria* deserializar_enviar_dato_memoria(void* buffer) {
    t_payload_enviar_dato_memoria* payload = malloc(sizeof(t_payload_enviar_dato_memoria));
    int desplazamiento = 0;
    memcpy(&(payload->direccion), buffer + desplazamiento, sizeof(payload->direccion));
    desplazamiento += sizeof(payload->direccion);

    memcpy(&(payload->tamDato), buffer + desplazamiento, sizeof(payload->tamDato));
    desplazamiento += sizeof(payload->tamDato);
    
    payload->dato = malloc(payload->tamDato);
    if (payload->dato == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }
    memcpy(payload->dato, buffer + desplazamiento, payload->tamDato);

    return payload;
}

void* serializar_solicitar_dato_memoria(t_payload_solicitar_dato_memoria* payload, int* size_payload) {
    *size_payload = sizeof(payload->direccion) + sizeof(payload->tam) + sizeof(payload->pid);
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &(payload->direccion), sizeof(payload->direccion));
    desplazamiento += sizeof(payload->direccion);
    memcpy(buffer + desplazamiento, &(payload->tam), sizeof(payload->tam));
    desplazamiento += sizeof(payload->tam);
    memcpy(buffer + desplazamiento, &(payload->pid), sizeof(payload->pid));

    return buffer;
}

t_payload_solicitar_dato_memoria* deserializar_solicitar_dato_memoria(void* buffer) {
    t_payload_solicitar_dato_memoria* payload = malloc(sizeof(t_payload_solicitar_dato_memoria));
    int desplazamiento = 0;
    memcpy(&(payload->direccion), buffer + desplazamiento, sizeof(payload->direccion));
    desplazamiento += sizeof(payload->direccion);
    memcpy(&(payload->tam), buffer + desplazamiento, sizeof(payload->tam));
    desplazamiento += sizeof(payload->tam);
    memcpy(&(payload->pid), buffer + desplazamiento, sizeof(payload->pid));
    return payload;
}

void* serializar_resultado_resize_memoria(t_payload_resultado_resize_memoria* payload, int* size_payload) {
    *size_payload = sizeof(payload->resultado);
    void* buffer = malloc(*size_payload);
    int desplazamiento = 0;
    memcpy(buffer + desplazamiento, &(payload->resultado), sizeof(payload->resultado));
    return buffer;
}

t_payload_resultado_resize_memoria* deserializar_resultado_resize_memoria(void* buffer) {
    t_payload_resultado_resize_memoria* payload = malloc(sizeof(t_payload_resultado_resize_memoria));
    int desplazamiento = 0;
    memcpy(&(payload->resultado), buffer + desplazamiento, sizeof(payload->resultado));
    return payload;
}
