#include "envios.h"

extern t_log *logger;

void enviar_paquete_entre(int socket, OP_CODES_ENTRE operacion, void *payload, int size_payload)
{
    t_paquete *paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = operacion;
    paquete->size_payload = size_payload;
    paquete->payload = payload;
    agregar_paquete_entre_a_paquete(paq, paquete);
    enviar_paquete(paq, socket);
    log_info(logger, "PAQUETE CREADO Y ENVIADO");
    eliminar_paquete(paq);
    free(paquete);
}