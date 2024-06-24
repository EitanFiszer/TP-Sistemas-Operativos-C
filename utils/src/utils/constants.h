#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "./PCB.h"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

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
  GET_INSTRUCCION, // memoria a cpu
	PEDIR_VALOR,
	INSTRUCCIONES_CARGADAS,
	//cpu kernel
	EXEC_PROCESO, //ejecuta esta pcb
	INTERRUMPIR_PROCESO, //DE KERNEL A CPU

	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
	IO_FS_CREATE,
	IO_FS_DELETE,

	IO_STDOUT_WRITE,
	IO_STDIN_READ,

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

// typedef struct {
// 	SYSCALL_INSTRUCCIONES instruccion;
// 	void* payload;
// }t_payload_syscall;

// typedef struct {
// 	OP_CODES_ENTRE operacion;
// 	void* payload;
// } t_paquete_entre;
typedef struct {
	OP_CODES_ENTRE operacion;
	int size_payload;
	void* payload;
} t_paquete_entre;

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
} t_payload_solicitar_dato_memoria;

typedef struct {
	int marco;
} t_payload_direccion_fisica;


typedef struct {
	int pid;
	int tam;
} t_payload_resize_memoria;

typedef struct {
	t_PCB pcb;
	int tam;
} t_payload_io_stdin_read;//cambie dato PCB ARREGLARLO EN OTROS LUGARES


//LOS SIGUIENTES PAYLOAD TIENEN UNA SERIALIZACION Y DESERIALIZACION
typedef struct {
	void* dato;
} t_payload_dato_memoria;

typedef struct {
	int direccion;
	void* dato;
} t_payload_enviar_dato_memoria;


typedef struct {
	t_PCB* pcb;
	char* recurso;
} t_payload_wait_signal;


// typedef struct {
// 	t_PCB* pcb;
// 	char* recurso;
// } t_payload_signal;
//UNI WAIT Y SIGNAL CAMBIARLO EN EL ARCHIVO


typedef struct {
	char* string;
} t_payload_recibir_string_io_stdin;

typedef struct {
	op_codes_io interfaz;
	char* op;
	int tiempo;
	//si es con tiempo aca 
} t_payload_instruccion_io;

typedef struct {
	char* path;
	int pid;
} t_payload_crear_proceso;

typedef struct {
	SYSCALL_INSTRUCCIONES instruccion;
	int tiempo;
	t_PCB* pcb;
	char* interfaz;
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



//no se si es un payload
typedef struct{
    op_codes_io op_code;
	char* interfaz;
    int tiempo;
	uint32_t direccion;
}op_io;



// t_payload_enviar_dato_memoria payload = { .direccion = 1234, .size_dato = 10 };
// payload.dato = malloc(payload.size_dato);
// memset(payload.dato, 0, payload.size_dato);  // Ejemplo de dato

// int size_payload;
// void* serialized_payload = serializar_payload_enviar_dato_memoria(&payload, &size_payload);

// t_paquete_entre paquete = { .operacion = OPERACION_ENVIAR_DATO_MEMORIA, .size_payload = size_payload, .payload = serialized_payload };

// enviar_paquete_entre(&paquete, socket_cliente);

// free(serialized_payload);
// free(payload.dato);


// t_paquete_entre* paquete_recibido = recibir_paquete_entre(socket_cliente);
// if (paquete_recibido->operacion == OPERACION_ENVIAR_DATO_MEMORIA) {
//     t_payload_enviar_dato_memoria* payload = deserializar_payload_enviar_dato_memoria(paquete_recibido->payload);
//     printf("Direccion: %d, Dato: %s\n", payload->direccion, (char*)payload->dato);

//     free(payload->dato);
//     free(payload);
// }

// free(paquete_recibido->payload);
// free(paquete_recibido);


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

#endif /* CONSTANTS_H */