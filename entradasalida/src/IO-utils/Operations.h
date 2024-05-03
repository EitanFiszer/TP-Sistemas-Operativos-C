typedef enum{
    IO_GEN_SLEEP
}op_codes;
typedef struct{
    op_codes op_code;
    void* tiempo;
}op;
