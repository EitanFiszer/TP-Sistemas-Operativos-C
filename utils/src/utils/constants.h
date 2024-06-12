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

	FS_TRUNCATE,
	FS_WRITE,
	FS_READ,
	FS_CREATE,
	FS_DELETE,

	IO_STDOUT_WRITE,

	//cpu memoria
	HANDSHAKE_CPU_MEMORIA,
	SOLICITAR_DIRECCION_FISICA,
	SOLICITAR_DATO_MEMORIA,
	ENVIAR_DATO_MEMORIA,
	RESIZE_MEMORIA,
	
	SYSCALL,
	WAIT,
	SIGNAL,
	INTERRUMPIO_PROCESO, // DE CPU A KERNEL, replanificar por interrupcion //PAYLOAD PCB
	ERROR_OUT_OF_MEMORY, // DE CPU A KERNEL, error de memoria
	TERMINO_EJECUCION,
	//entrada salida kernel
	CONEXION_IO,

	INSTRUCCION_IO,
	IO_INTERFAZ_CREADA,

	EXIT_PROCESS

} OP_CODES_ENTRE;

typedef enum {
	IO_GEN_SLEEP,
	IO_STDIN_READ,
	IO_STDOUT_READ,
	IO_STDOUT_WRITE,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
} SYSCALL_INSTRUCCIONES;

typedef struct {
	SYSCALL_INSTRUCCIONES instruccion;
	void* payload;
}t_payload_syscall;

typedef struct {
	OP_CODES_ENTRE operacion;
	void* payload;
} t_paquete_entre;

typedef struct {
    int pid;
    char* path;
} t_payload_crear_proceso;

typedef struct {
    int pid;
} t_payload_finalizar_proceso;

typedef struct {
    int PID;
    int program_counter;
} t_payload_pc_a_instruccion;

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
	t_PCB* pcb;
	char* recurso;
} t_payload_wait;

typedef struct {
	t_PCB* pcb;
	char* recurso;
} t_payload_signal;

typedef struct {
	t_PCB* pcb;
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

typedef struct{
	char* interfaz;
	char* regDire;
	char* regTam;
}t_payload_io_stdout_write;

typedef struct{
	char* interfaz;
	char* nombreArchivo;
}t_payload_fs_create;

typedef struct{
	char* interfaz;
	char* nombreArchivo;
	char* regTam;
}t_payload_fs_truncate;

typedef struct{
	char* interfaz;
	char* nombreArchivo;
	char* regTam;
	char* regDire;
	char* regPuntero;
}t_payload_fs_writeORread;

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
