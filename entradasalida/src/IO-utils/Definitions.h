typedef enum{
    GENERICA, 
    STDIN,
    STDOUT, 
    DIALFS
}tipo_interfaz;
typedef struct{
    char* nombre;
    tipo_interfaz tipo;
}t_interfaz;