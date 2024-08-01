#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stddef.h>
#include "./PCB.h"

typedef enum {
    MENSAJE,
    PAQUETE
} op_code;

typedef enum {
    CPU,
    KERNEL,
    IO,
    MEMORIA
} ID;

typedef enum {
    IO_GEN,
    IO_STDIN,
    IO_STDOUT,
    IO_FS
} op_codes_io;

typedef struct {
    int socket;
    int tam_pagina;
} handshake_cpu_memoria;

typedef enum {
    // kernel memoria
    CREAR_PROCESO,
    ARCHIVO_NO_ENCONTRADO,  // DE Memoria A Kernel

    FINALIZAR_PROCESO,
    AMPLIAR_PROCESO,
    REDUCIR_PROCESO,
    PC_A_INSTRUCCION,
    GET_INSTRUCCION,       // memoria a cpu
    FIN_DE_INSTRUCCIONES,  // memoria a cpu
    PEDIR_VALOR,
    INSTRUCCIONES_CARGADAS,
    // cpu kernel
    EXEC_PROCESO,         // ejecuta esta pcb
    INTERRUMPIR_PROCESO,  // DE KERNEL A CPU

    IO_GEN_SLEEP,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    IO_FS_CREATE,
    IO_FS_DELETE,

    IO_STDOUT_WRITE,
    IO_STDIN_READ,

    // cpu memoria
    HANDSHAKE_CPU_MEMORIA,
    SOLICITAR_DIRECCION_FISICA,
    SOLICITAR_DATO_MEMORIA,
    ENVIAR_DATO_MEMORIA,
    RESIZE_MEMORIA,
    DIRECCION_FISICA,
    DATO_MEMORIA,

    WAIT,
    SIGNAL,
    INTERRUMPIO_PROCESO,  // DE CPU A KERNEL, replanificar por interrupcion //PAYLOAD PCB
    ERROR_OUT_OF_MEMORY,  // DE CPU A KERNEL, error de memoria
    RESIZE_SUCCESS,       
    TERMINO_EJECUCION,
    TERMINE_OPERACION, //DE IO A KERNEL, termino io de hacer operacion
    // entrada salida kernel
    CONEXION_IO,

    INSTRUCCION_IO,
    ESCRIBIR_MEMORIA,

    IO_INTERFAZ_CREADA,

    EXIT_PROCESS,
    
    CONFIRMAR_SYSCALL // DE KERNEL A CPU, confirmacion de wait o signal

} OP_CODES_ENTRE;

typedef struct {
    int bloque_inicial;
    int tam_archivo;
} t_metadata;

typedef struct {
    t_metadata* map;
    size_t size;
    int fd;
    t_metadata metadata;
} t_diccionario;

typedef enum {
    GEN_SLEEP,
    STDIN_READ,
    STDOUT_READ,
    STDOUT_WRITE,
    FS_CREATE,
    FS_DELETE,
    FS_TRUNCATE,
    FS_WRITE,
    FS_READ,
} SYSCALL_INSTRUCCIONES;

// GENERALES
typedef struct {
    OP_CODES_ENTRE operacion;
    int size_payload;
    void* payload;
} t_paquete_entre;

// CPU - MEMORIA
typedef struct {
    char* instruccion;
} t_payload_get_instruccion;

typedef struct {
    int PID;
    int program_counter;
} t_payload_pc_a_instruccion;

typedef struct {
    int PID;
    int pagina;
} t_payload_solicitar_direccion_fisica;

typedef struct {
    int direccion;
    int tam;
} t_payload_solicitar_dato_memoria;

typedef struct {
    int marco;
} t_payload_direccion_fisica;

typedef struct {
    int pid;
    int tam;
} t_payload_resize_memoria;

// CPU - KERNEL
typedef struct {
    int tam;
    t_PCB* pcb;
    char* interfaz;
    int dirFisica;
} t_payload_io_stdin_read;  // cambiar en instrucciones

typedef struct {
  int resultado;
} t_payload_resultado_resize_memoria;

typedef struct {
    t_PCB* pcb;
    char* recurso;
} t_payload_wait_signal;

typedef struct {
    char* interfaz;
    int tiempo;
    t_PCB* pcb;
} t_payload_io_gen_sleep;

typedef struct {
    char* interfaz;
    int direccionFisica;
    int tam;
    t_PCB* pcb;
} t_payload_io_stdout_write;

typedef struct {
    char* interfaz;
    char* nombreArchivo;
    t_PCB* pcb;
} t_payload_fs_create;  // hacer serializacion

typedef struct {
    char* interfaz;
    char* nombreArchivo;
    int tam;
    t_PCB* pcb;
} t_payload_fs_truncate;  // hacer serializacion

typedef struct {
    char* interfaz;
    char* nombreArchivo;
    int tam;
    int punteroArchivo;
    int dirFisica;
    t_PCB* pcb;
} t_payload_fs_writeORread;

// IO - KERNEL
typedef struct {
    char* interfaz;
    int direccionFisica;
    int tam;
} t_payload_io_stdin_read_de_kernel_a_io;

typedef struct {
    char* nombre;
    char* tipo_interfaz;
} t_payload_interfaz_creada;

// KERNEL - MEMORIA
typedef struct {
    char* path;
    int pid;
} t_payload_crear_proceso;

// CPU/IO - MEMORIA
typedef struct {
    int direccion;
    void* dato;
    int tamDato;
} t_payload_enviar_dato_memoria;

typedef struct {
    int direccion;
    void* dato;
    int size_cadena;
    int pid;
} t_payload_escribir_memoria;

typedef struct {
    int direccion;
    int size_cadena;
} t_payload_leer_memoria;

// LOS SIGUIENTES PAYLOAD TIENEN UNA SERIALIZACION Y DESERIALIZACION
typedef struct {
    void* dato;
} t_payload_dato_memoria;

typedef struct {
    char* string;
    int size_string;
} t_payload_recibir_string_io_stdin;

typedef struct {
    char* interfaz;
    char* op;
    int tiempo;
} t_payload_instruccion_io;


#endif /* CONSTANTS_H */