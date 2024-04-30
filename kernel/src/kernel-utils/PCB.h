//ESTRUCTURA PCB
typedef struct{
    int PID;
    uint32_t program_counter;
    int quantum;
    CPU_Registro cpu_registro; //REGISTRO?
    estado_proceso estado;
}PCB;