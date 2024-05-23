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
	
	SYSCALL,
	INTERRUMPIO_PROCESO, // DE CPU A KERNEL, replanificar por interrupcion
	TERMINO_EJECUCION,
	//entrada salida kernel
	CONEXION_IO,

	INSTRUCCION_IO

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
