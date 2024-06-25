#include "conexiones.h"

extern t_log* logger;
extern int socketMemoria;
extern int socketKernel;

void enviar_pcb_kernel(t_PCB *pcb, OP_CODES_ENTRE operacion) {
    int size_pcb;
    void* pcbSerializado = serializar_pcb(pcb, &size_pcb);
    enviar_paquete_entre(socketMemoria, operacion, pcbSerializado, size_pcb);

    // t_paquete *paq = crear_paquete();
    // t_paquete_entre *paquete=malloc(sizeof(t_paquete_entre));  

    // paquete->operacion = operacion;
    // paquete->payload = pcb;
    // paquete->size_payload = sizeof(t_PCB);
    
    // agregar_paquete_entre_a_paquete(paq, paquete);
    // enviar_paquete(paq, socketKernel);
    // eliminar_paquete(paq);
    // free(paquete);
}

handshake_cpu_memoria handshake_memoria(char* ip_memoria, char* puerto_memoria) {
    handshake_cpu_memoria handData = { .socket = -1, .tam_pagina = -1 };
    
    handData.socket = crear_conexion(ip_memoria, puerto_memoria);

    if (handData.socket == -1) {
        log_error(logger, "No se pudo conectar con la memoria");
        return handData;
    } else {
        log_info(logger, "Conexión establecida con la memoria");
    }

    uint32_t modulo = CPU;

    send(handData.socket, &modulo, sizeof(uint32_t), 0);
    recv(handData.socket, &handData.tam_pagina, sizeof(uint32_t), 0);

    return handData;
}

void* solicitar_dato_memoria(int dirFisica) {
    t_payload_solicitar_dato_memoria* payload = malloc(sizeof(t_payload_solicitar_dato_memoria));
    payload->direccion = dirFisica;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = SOLICITAR_DATO_MEMORIA;
    paquete->payload = payload;
    paquete->size_payload = sizeof(t_payload_solicitar_dato_memoria);

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketMemoria);

   
    t_paquete_entre* paqueteRecibidoEntero = recibir_paquete_entre(socketMemoria);
    if (paqueteRecibidoEntero == NULL) {
        return NULL;
    }
//ver esto pq depende del lato la serializacion
    t_payload_dato_memoria* payloadRecibido = paqueteRecibidoEntero->payload;
    void* dato = payloadRecibido->dato;

    return dato;
}

int enviar_dato_memoria(int dirFisica, void* dato) {

//depende del dato la serializacion


    t_payload_enviar_dato_memoria* payload = malloc(sizeof(t_payload_enviar_dato_memoria));
    payload->direccion = dirFisica;
    payload->dato = dato;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = ENVIAR_DATO_MEMORIA;
    paquete->payload = payload;

    t_paquete* paq = crear_paquete();
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketMemoria);

    t_list* paqueteRecibido = recibir_paquete(socketMemoria);
    if (paqueteRecibido == NULL) {
        return -1;
    }

    t_paquete_entre* paqueteRecibidoEntero = list_get(paqueteRecibido, 0);
    if (paqueteRecibidoEntero == NULL) {
        return -1;
    }

    t_payload_dato_memoria* payloadRecibido = paqueteRecibidoEntero->payload;
    void* resultado = payloadRecibido->dato;

    if (resultado == NULL) {
        return -1;
    }

    return *(int*)resultado;
}

int solicitar_resize_memoria(int pid, int tam) {

    t_payload_resize_memoria* payload = malloc(sizeof(t_payload_resize_memoria));
    payload->pid = pid;
    payload->tam = tam;


    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = RESIZE_MEMORIA;
    paquete->payload = payload;
    paquete->size_payload = sizeof(t_payload_resize_memoria);

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketMemoria);
    eliminar_paquete(paq);

    
    t_paquete_entre* paqueteRecibidoEntero = recibir_paquete_entre(socketMemoria);
    if (paqueteRecibidoEntero == NULL) {
        return -1;
    }

    t_payload_resize_memoria* payloadRecibido = (t_payload_resize_memoria*)paqueteRecibidoEntero->payload;
    int resultado = payloadRecibido->tam;

    return resultado;
}
//ACA TIENEN QUE ENVIAR TAMBIEN LA PCB
void solicitar_wait(char* recurso) {
    t_payload_wait_signal* payload = malloc(sizeof(t_payload_wait_signal));
    payload->recurso = recurso;
    // payload->pcb = pcb;
    int size_payload;
    void* buffer = serializar_wait_signal(payload,&size_payload);

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = WAIT;
    paquete->payload = buffer;
    paquete->size_payload = size_payload;

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketKernel);

    eliminar_paquete(paq);
    free(payload);
}


//ACA ENVIAR TAMBIEN LA PCB
void solicitar_signal(char* recurso) {
     t_payload_wait_signal* payload = malloc(sizeof(t_payload_wait_signal));
    payload->recurso = recurso;
    // payload->pcb = pcb;
    int size_payload;
    void* buffer = serializar_wait_signal(payload,&size_payload);

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = SIGNAL;
    paquete->payload = buffer;
    paquete->size_payload = size_payload;

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketKernel);
    eliminar_paquete(paq);
}
//ACA ENVIAR TAMBIEN PCB
char* solicitar_io_stdin(int tam) {
    t_payload_io_stdin_read* payload = malloc(sizeof(t_payload_io_stdin_read));
    payload->tam = tam;
    // payload->pcb = pcb;

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = IO_STDIN_READ;
    paquete->payload = payload;
    
    paquete->size_payload = sizeof(t_payload_io_stdin_read);

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketKernel);
    eliminar_paquete(paq);


    t_paquete_entre* paqueteRecibidoEntero = recibir_paquete_entre(socketKernel);
    if (paqueteRecibidoEntero == NULL) {
        return NULL;
    }

    t_payload_recibir_string_io_stdin* payloadRecibido = deserializar_recibir_string_io_stdin(paqueteRecibidoEntero->payload);
    char* resultado = payloadRecibido->string;
    return resultado;
}
//ACA ENVIAR TAMBIEN PCB
void solicitar_io_stdout(char* interfaz, char* regDire, char* regTam){
    t_payload_io_stdout_write* payload = malloc(sizeof(t_payload_io_stdout_write));
    payload->interfaz = interfaz;
    payload->regDire = regDire;
    payload->regTam = regTam;
    // payload->pcb = pcb;
    int size_payload;
    void* buffer =serializar_io_stdout_write(payload,&size_payload);

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = IO_STDOUT_WRITE;
    paquete->payload = buffer;
    paquete->size_payload = size_payload;

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketKernel);

    eliminar_paquete(paquete);
}

void solicitar_fs_createORdelete(char* interfaz, char* nombreArchivo, OP_CODES_ENTRE oper){
    t_payload_fs_create* payload = malloc(sizeof(t_payload_fs_create));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    int size_payload;
    void* buffer = serializar_fs_create(payload,&size_payload);

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = oper;
    paquete->size_payload = size_payload;
    paquete->payload = buffer;

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    enviar_paquete(paq, socketKernel);
    eliminar_paquete(paq);
}


void solicitar_fs_truncate(char* interfaz, char* nombreArchivo, char* regTam){
    t_payload_fs_truncate* payload = malloc(sizeof(t_payload_fs_truncate));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->regTam = regTam;

    int size_payload;
    void* buffer = serializar_fs_truncate(payload, &size_payload);


    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = IO_FS_TRUNCATE;
    paquete->size_payload = size_payload;
    paquete->payload = buffer;

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, socketKernel);
    eliminar_paquete(paq);
}

void solicitar_fs_writeORread(char* interfaz, char* nombreArchivo, char* regTam, char* regDire, char* regPuntero, OP_CODES_ENTRE oper){
    t_payload_fs_writeORread* payload = malloc(sizeof(t_payload_fs_writeORread));
    payload->interfaz = interfaz;
    payload->nombreArchivo = nombreArchivo;
    payload->regTam = regTam;
    payload->regDire = regDire;
    payload->regPuntero = regPuntero;

    int size_payload;
    void* buffer = serializar_fs_writeORread(payload, &size_payload);

    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = oper;
    paquete->size_payload =size_payload;
    paquete->payload = buffer;

    t_paquete* paq = crear_paquete();
    agregar_paquete_entre_a_paquete(paq, paquete);

    eliminar_paquete(paq);
    enviar_paquete(paq, socketKernel);
}


