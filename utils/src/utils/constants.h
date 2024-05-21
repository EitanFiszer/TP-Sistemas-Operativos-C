typedef enum {
	CPU,
	KERNEL,
	IO,
	MEMORIA
}ID;

typedef enum {
	CREAR_PROCESO,
	FINALIZAR_PROCESO,
	AMPLIAR_PROCESO,
	REDUCIR_PROCESO,
	PC_A_INSTRUCCION,
	PEDIR_VALOR,
	INSTRUCCIONES_CARGADAS,
	EXEC_PROCESO
} OP_CODES_ENTRE;


typedef struct {
	OP_CODES_ENTRE operacion;
	void* payload;
} t_paquete_entre;

typedef struct {
    int PID;
    uint32_t program_counter;
} t_payload_fetch_instruccion;

typedef struct {
	char* path;
	int pid;
} payload_crear_proceso;

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