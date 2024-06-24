#include "client.h"

void *serializar_paquete(t_paquete *paquete, int bytes) {
    void *magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic;
}
void *serializar_paquete_entre(t_paquete_entre *paquete, int *bytes) {
    int desplazamiento = 0;
    int size_payload = paquete->size_payload;

    *bytes = sizeof(OP_CODES_ENTRE) + sizeof(int) + size_payload;
    void *magic = malloc(*bytes);

    memcpy(magic + desplazamiento, &(paquete->operacion), sizeof(OP_CODES_ENTRE));
    desplazamiento += sizeof(OP_CODES_ENTRE);
    memcpy(magic + desplazamiento, &(paquete->size_payload), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, paquete->payload, size_payload);

    return magic;
}
int crear_conexion(char *ip, char *puerto) {
    int err;
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    err = getaddrinfo(ip, puerto, &hints, &server_info);
    if (err < 0) {
        perror("OCURRIO UN ERROR AL OBTENER INFORMACION");
        exit(EXIT_FAILURE);
    }
    // Ahora vamos a crear el socket.
    int socket_cliente = 0;

    // Ahora que tenemos el socket, vamos a conectarlo
    socket_cliente = socket(server_info->ai_family,
                            server_info->ai_socktype,
                            server_info->ai_protocol);
    err = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

    freeaddrinfo(server_info);
    if (err < 0) {
        perror("OCURRIO UN ERROR AL INTENTAR CONECTARSE");
        exit(EXIT_FAILURE);
    }
    return socket_cliente;
}

void enviar_mensaje(char *mensaje, int socket_cliente) {
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    int bytes = paquete->buffer->size + 2 * sizeof(int);

    void *a_enviar = serializar_paquete(paquete, bytes);

    ssize_t err = send(socket_cliente, a_enviar, bytes, 0);
    if (err < 0) {
        perror("OCURRIO UN ERROR AL ENVIAR MENSAJE");
        exit(EXIT_FAILURE);
    }
    free(a_enviar);
    eliminar_paquete(paquete);
}

void crear_buffer(t_paquete *paquete) {
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = 0;
    paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(void) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    paquete->codigo_operacion = PAQUETE;
    crear_buffer(paquete);
    return paquete;
}

void agregar_paquete_entre_a_paquete(t_paquete *paquete, t_paquete_entre *paquete_entre) {
    int size_paquete_entre;
    void *buffer_paquete_entre = serializar_paquete_entre(paquete_entre, &size_paquete_entre);

    int new_size = paquete->buffer->size + size_paquete_entre + sizeof(int);
    paquete->buffer->stream = realloc(paquete->buffer->stream, new_size);

    memcpy(paquete->buffer->stream + paquete->buffer->size, &size_paquete_entre, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), buffer_paquete_entre, size_paquete_entre);

    paquete->buffer->size = new_size;

    free(buffer_paquete_entre);
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio) {
    // log_info(logger,"AGREGANDO A PAQUETE");
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente) {
    // log_info(logger,"ENVIANDO PAQUETE PAQUETE");
    int bytes = paquete->buffer->size + 2 * sizeof(int);
    // log_info(logger,"BYTES: %d",bytes );
    void *a_enviar = serializar_paquete(paquete, bytes);

    ssize_t err = send(socket_cliente, a_enviar, bytes, 0);
    if (err < 0) {
        perror("OCURRIO UN ERROR AL ENVIAR PAQUETE");
        exit(EXIT_FAILURE);
    }
    free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete) {
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void liberar_conexion(int socket_cliente) {
    close(socket_cliente);
}

int connectAndHandshake(char *ip, char *puerto, ID modulo, char *dest, t_log *logger) {
    log_info(logger, "CONNECT & HANDSHAKE EN PUERTO %s", puerto);
    int conexion = crear_conexion(ip, puerto);

    if (conexion < 0) {
        log_error(logger, "No se pudo conectar a %s!", dest);
        return -1;
    }

    uint32_t handshake = modulo;
    uint32_t result;
    send(conexion, &handshake, sizeof(uint32_t), 0);
    recv(conexion, &result, sizeof(uint32_t), MSG_WAITALL);
    
    if (result == -1) {
        log_error(logger, "No se pudo conectar a %s!", dest);
        return -1;
    } else {
        log_info(logger, "Conectado a %s %s:%s -- %d", dest, ip, puerto, conexion);
    }

    return conexion;
}