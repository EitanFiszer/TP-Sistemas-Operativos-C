#include <kernel-utils/PCB.h>
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
// typedef struct {
// 	ID modulo;
// 	int socket;
// } handshake_t;

t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* cola_exec;

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

void iniciar_colas(void);
void leer_configs();
void consola_interactiva(void);
void LTS(void);
void STS(void);
t_PCB* crear_PCB(int PID);
int g_multiprogracion_actual (void);