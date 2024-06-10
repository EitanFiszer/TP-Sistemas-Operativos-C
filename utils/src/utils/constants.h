#pragma once

#include "./PCB.h"


typedef enum {
	CPU,
	KERNEL,
	IO,
	MEMORIA
}ID;

typedef enum{
    IO_GEN,
    IO_STDIN,
    IO_STDOUT,
    IO_FS
}op_codes_io;
typedef struct{
    op_codes_io op_code;
	char* interfaz;
    void* tiempo;
}op_io;

typedef struct {
	int socket;
	int tam_pagina;
} handshake_cpu_memoria;


typedef enum {
	//kernel memoria
	CREAR_PROCESO,
	FINALIZAR_PROCESO,
	AMPLIAR_PROCESO,
	REDUCIR_PROCESO,
	PC_A_INSTRUCCION,
	PEDIR_VALOR,
	INSTRUCCIONES_CARGADAS,
	//cpu kernel
	EXEC_PROCESO, //ejecuta esta pcb
	INTERRUMPIR_PROCESO, //DE KERNEL A CPU

	//cpu memoria
	HANDSHAKE_CPU_MEMORIA,
	SOLICITAR_DIRECCION_FISICA,
	SOLICITAR_DATO_MEMORIA,
	ENVIAR_DATO_MEMORIA,
	RESIZE_MEMORIA,
	
	SYSCALL,
	INTERRUMPIO_PROCESO, // DE CPU A KERNEL, replanificar por interrupcion
	ERROR_OUT_OF_MEMORY, // DE CPU A KERNEL, error de memoria
	WAIT,
	SIGNAL,
	IO_STDIN_READ,
	TERMINO_EJECUCION,
	//entrada salida kernel
	CONEXION_IO,

	INSTRUCCION_IO,
	IO_INTERFAZ_CREADA

} OP_CODES_ENTRE;

typedef enum {
	IO_GEN_SLEEP
} SYSCALL_INSTRUCCIONES;

typedef struct {
	OP_CODES_ENTRE operacion;
	void* payload;
} t_paquete_entre;

typedef struct {
    int PID;
    uint32_t program_counter;
} t_payload_fetch_instruccion;

typedef struct {
	int PID;
	int pagina;
} t_payload_solicitar_direccion_fisica;

typedef struct {
	int direccion;
} t_payload_solicitar_dato_memoria;

typedef struct {
	void* dato;
} t_payload_dato_memoria;

typedef struct {
	int direccion;
	void* dato;
} t_payload_enviar_dato_memoria;

typedef struct {
	int marco;
} t_payload_direccion_fisica;

typedef struct {
	int pid;
	int tam;
} t_payload_resize_memoria;

typedef struct {
	char* recurso;
} t_payload_wait;

typedef struct {
	char* recurso;
} t_payload_signal;

typedef struct {
	int tam;
} t_payload_io_stdin_read;

typedef struct {
	char* string;
} t_payload_recibir_string_io_stdin;

typedef struct {
	op_codes_io interfaz;
	char* op;
	int tiempo;
	//si es con tiempo aca 
} payload_intruccion_io;

typedef struct {
	char* path;
	int pid;
} payload_crear_proceso;

typedef struct {
	SYSCALL_INSTRUCCIONES instruccion;
	char* interfaz;
	int tiempo;
	t_PCB* pcb;
} t_payload_io_gen_sleep;

typedef struct {
	char* nombre;
	char* tipo_interfaz;
} t_payload_interfaz_creada;



/*
typedef struct {
	char* path;
	int pid;
} payload_crear_proceso;

t_paquete* crear_paquete();
t_paquete_entre* instruccion;
instruccion = malloc(sizeof(t_paquete_entre));
instruccion->operacion = CREAR_PROCESO;

payload_crear_proceso* payload = malloc(sizeof(payload_crear_proceso));
payload->path = "test.txt";
payload->pid = 1;

instruccion->payload = payload;

agregar_a_paquete(paquete, instruccion, sizeof(t_paquete_entre));
*/
