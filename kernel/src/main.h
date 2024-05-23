#include <utils/PCB.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h> 
#include <utils/server.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <kernel-utils/conexiones.h>
#include <semaphore.h>
#include <readline/readline.h>


// typedef struct {
// 	ID modulo;
// 	int socket;
// } handshake_t;

t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* cola_exec;

pthread_mutex_t sem_q_new;
pthread_mutex_t sem_q_ready;
pthread_mutex_t sem_q_blocked;
pthread_mutex_t sem_q_exit;
pthread_mutex_t sem_q_exec;
sem_t sem_sts_cpu_libre;

sem_t sem_lts_proceso_cargado; //en duda
sem_t sem_cont_ready;

// sem_t cont_exit;


t_log *logger;
t_config *config;

int puerto;
char* ip_memoria;
char* ip_cpu;
char* puerto_memoria;
char* puerto_escucha;
char* puerto_cpu_dispatch;
char* puerto_cpu_interrupt;
int resultHandshakeMemoria;
int resultHandshakeDispatch;
int resultHandshakeInterrupt;
int server_fd;
int quantum;

//INT ULTIMO PROCESO CREADO
int PID = 0;



void leer_configs();

void iniciar_colas(void);
void iniciar_semaforos(void);

void consola_interactiva(void*);
void LTS_N_R(void);
void STS(void);
// void desalojar(t_PCB*);

//FUNCIONES DE CONEXION
void esperar_paquetes_cpu_dispatch(void);
void esperar_paquetes_memoria(void);
void enviar_instrucciones_memoria(char*);
void enviar_paquete_cpu_dispatch (OP_CODES_ENTRE,void*);
void interrumpir(void);

t_PCB* crear_PCB(int);
int g_multiprogracion_actual (void);
void iniciar_proceso(char*){


