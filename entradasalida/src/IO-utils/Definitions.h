typedef enum{
    IO_GEN, 
    IO_STDIN,
    IO_STDOUT, 
    DIALFS
}tipo_interfaz;
typedef struct{
    char* nombre;
    tipo_interfaz tipo;
}t_interfaz;