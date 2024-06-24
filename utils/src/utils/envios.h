#include "./constants.h"
#include <commons/log.h>
#include "./client.h"

void enviar_paquete_entre(int socket, OP_CODES_ENTRE operacion, void *payload, int size_payload);