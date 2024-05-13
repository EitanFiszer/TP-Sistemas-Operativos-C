#include <kernel-utils/PCB.h>

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

t_PCB crear_PCB(int PID);

//INT ULTIMO PROCESO CREADO
int PID = 0;
