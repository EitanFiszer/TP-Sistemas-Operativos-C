t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* cola_exec;

t_log *logger;
t_config *config;
char* ip_memoria;
char* ip_cpu;
char* puerto_memoria;
char* puerto_escucha;
char* puerto_cpu_dispatch;
char* puerto_cpu_interrupt;
