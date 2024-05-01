typedef enum{
    NEW,
    READY, 
    BLOCKED,
    EXEC,
    EXIT
}estado_proceso;
typedef enum{
    INTERRUPT, //NO ESTOY SEGURA
    WAIT,
    SIGNAL,
}motivo_desalojo;