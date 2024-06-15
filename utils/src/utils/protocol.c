#include <utils/client.h>
#include <utils/server.h>
#include "protocol.h"

t_paquete_entre* crear_paquete_entre(OP_CODES_ENTRE operacion, void* payload, int size_payload)
{
    t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));    
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;

    // Asignamos memoria para el payload y copiamos los datos
    paquete->payload = malloc(size_payload);
    memcpy(paquete->payload, &payload, size_payload);

    return paquete;
}

void enviar_paquete_entre(t_paquete_entre* paquete_entre, int socket_cliente, void*(serializar_payload)(void*, int, int*))
{
    int bytes;
    void* a_enviar = serializar_paquete_entre(paquete_entre, &bytes, serializar_payload);
    t_paquete* paquete = crear_paquete();
    agregar_a_paquete(paquete, a_enviar, bytes);
    enviar_paquete(paquete, socket_cliente);
}

t_paquete_entre* recibir_paquete_entre(int socket_cliente, void*(deserializar)(void*, int))
{
    t_list* paquete = recibir_paquete(socket_cliente);
    if (paquete == NULL) {
        return NULL;
    }

    t_buffer* buffer = list_get(paquete, 0);

    return deserializar_paquete_entre(buffer, buffer->size - sizeof(int), deserializar);
}

void* serializar_paquete_entre(t_paquete_entre* paquete_entre, int* bytes, void*(serializar_payload)(void* payload, int desplazamiento, int* bytes))
{
    *bytes = paquete_entre->size_payload;
    void* magic = malloc(*bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(paquete_entre->operacion), sizeof(int));
    desplazamiento += sizeof(int);
    void* newMagic = serializar_payload(paquete_entre->payload, desplazamiento, bytes);

    // merge 2 magics
    memcpy(magic + desplazamiento, newMagic, *bytes - sizeof(int));


    return magic;
}

t_paquete_entre* deserializar_paquete_entre(void* stream, int size_payload, void*(deserializar)(void*, int))
{
    t_paquete_entre* paquete_entre = malloc(sizeof(t_paquete_entre));
    int desplazamiento = 0;

    memcpy(&(paquete_entre->operacion), stream, sizeof(int));
    desplazamiento += sizeof(int);

    paquete_entre->size_payload = size_payload;
   
    paquete_entre->payload = deserializar(stream + desplazamiento, size_payload);

    return paquete_entre;
}
