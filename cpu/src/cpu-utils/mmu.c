#include <commons/collections/list.h>
#include <utils/constants.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/envios.h>
#include <math.h>
#include "TLB.h"

/*
A la hora de traducir direcciones lógicas a físicas, la CPU debe tomar en cuenta que el esquema de memoria del sistema es de Paginación. 
Por lo tanto, las direcciones lógicas se compondrán de la siguiente manera:
[número_pagina | desplazamiento]

número_página = floor(dirección_lógica / tamaño_página)
desplazamiento = dirección_lógica - número_página * tamaño_página
*/

extern t_list* TLB;
extern int TAM_PAGINA;
extern int socketMemoria;
extern int TLB_MAX_SIZE;

int buscarEnTLB(int PID, int numeroPagina) {
    for (int i = 0; i < list_size(TLB); i++) {
        tlb_entry* entrada = list_get(TLB, i);
        if (entrada->pagina == numeroPagina && entrada->pid == PID) {
            return entrada->marco;
        }
    }
    return -1;
}

//if -1 --> buscarEnTablaDePaginas  else --> direccionFisica

int buscarEnTablaDePaginas(int PID, int numeroPagina) {
    t_payload_solicitar_direccion_fisica payload = {
        .PID = PID,
        .pagina = numeroPagina
    };
    enviar_paquete_entre(socketMemoria, SOLICITAR_DIRECCION_FISICA, &payload, sizeof(t_payload_solicitar_direccion_fisica));
    
    t_paquete_entre* paqueteRecibido = recibir_paquete_entre(socketMemoria);

    if (paqueteRecibido == NULL || paqueteRecibido->operacion != DIRECCION_FISICA) {
        return -1;
    }

    t_payload_direccion_fisica* payloadRecibido = paqueteRecibido->payload;

    log_info(logger, "PID: %d - OBTENER MARCO - Pagina: %d - Marco: %d", PID, numeroPagina, payloadRecibido->marco);

    return payloadRecibido->marco;
}

int calcularDireccionFisica(int PID, int direccionLogica) {
    int numeroPagina = (int)floor(direccionLogica / TAM_PAGINA);
    int desplazamiento = direccionLogica - numeroPagina * TAM_PAGINA;

    int marco = buscarEnTLB(PID, numeroPagina);
    if (marco == -1) {
        log_info(logger, "PID: %d - TLB MISS - Pagina %d", PID, numeroPagina);
        marco = buscarEnTablaDePaginas(PID, numeroPagina);

        if(TLB_MAX_SIZE > 0) {
          agregarEntradaTLB(PID, numeroPagina, marco);
        }
    } else {
        log_info(logger, "PID %d - TLB HIT - Pagina %d", PID, numeroPagina);
    }

    if (marco == -1) {
        return -1;
    }

    return marco * TAM_PAGINA + desplazamiento;
}
