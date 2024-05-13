typedef enum {
    NEW,
    READY, 
    BLOCKED,
    EXEC,
    EXIT
}t_proceso_estado;

typedef enum{
    INTERRUPT, //NO ESTOY SEGURA
    WAIT,
    SIGNAL,
} t_motivo_desalojo;