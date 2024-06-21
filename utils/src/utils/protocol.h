// #ifndef PROTOCOL_H
// #define PROTOCOL_H

// t_paquete_entre* crear_paquete_entre(OP_CODES_ENTRE operacion, void* payload, int size_payload);
// t_paquete_entre* recibir_paquete_entre(int socket_cliente, void*(deserializar)(void*, int));
// t_paquete_entre* deserializar_paquete_entre(void* stream, int size_payload, void*(deserializar)(void*, int));
// void* serializar_paquete_entre(t_paquete_entre* paquete_entre, int* bytes, void*(serializar_payload)(void*, int, int*));
// void enviar_paquete_entre(t_paquete_entre* paquete_entre, int socket_cliente, void*(serializar_payload)(void*, int, int*));

// #endif