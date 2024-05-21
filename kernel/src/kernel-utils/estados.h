typedef enum {
    NEW,
    READY, 
    BLOCKED,
    EXEC,
    EXIT
}t_proceso_estado;

typedef enum{
    INTERRUPT,
    WAIT,
    SIGNAL,
    FINISH
} t_motivo_desalojo;