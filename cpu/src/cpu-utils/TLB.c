#include "TLB.h"

#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>

extern char* TLB_ALGORITMO_REEMPLAZO;
extern int TLB_MAX_SIZE;
extern t_list* TLB;
extern t_log* logger;

int punteroFifo = 0;
t_list* lru_peticiones = NULL;

int marcoSegunPIDyPagina(int pid, int pagina) {
    int marco = -1;
    bool encontrado = false;
    for (int i = 0; i < list_size(TLB) && !encontrado; i++) {
        tlb_entry* entrada = list_get(TLB, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            marco = entrada->marco;
            encontrado = true;
        }
    }
    return marco;
}

bool tlbLlena() {
    return list_size(TLB) == TLB_MAX_SIZE;
}

void agregarEntradaTLB(int pid, int pagina, int marco) {
    if (tlbLlena()) {
        // ver por algoritmo de reemplazo
        if (string_equals_ignore_case(TLB_ALGORITMO_REEMPLAZO, "FIFO")) {
            TLBagregarFIFO(pid, pagina, marco);
        } else if (string_equals_ignore_case(TLB_ALGORITMO_REEMPLAZO, "LRU")) {
            TLBagregarFIFO(pid, pagina, marco);
        } else {
            // no debería llegar acá
            log_error(logger, "Algoritmo de reemplazo de TLB no válido");
            return;
        }
    } else {
        tlb_entry* entrada = malloc(sizeof(tlb_entry));
        entrada->pid = pid;
        entrada->pagina = pagina;
        entrada->marco = marco;
        list_add(TLB, entrada);
    }

    // crear lista de peticiones y actualizarla
    if (string_equals_ignore_case(TLB_ALGORITMO_REEMPLAZO, "LRU")) {
        if (lru_peticiones == NULL) {
            lru_peticiones = list_create();
        }
        actualizarTimestampOAgregarAPeticiones(pid, pagina);
    }
}

bool compareTimestamp(void* a, void* b) {
    return ((tlb_entry_lru*)a)->timestamp < ((tlb_entry_lru*)b)->timestamp;
}

// fifo: sacar el primero y agregar al final
void TLBagregarFIFO(int pid, int pagina, int marco) {
    tlb_entry* entrada = malloc(sizeof(tlb_entry));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;

    // log_info(logger, "puntero antes de reemplazo: %d", punteroFifo);

    if(list_size(TLB) > 0) {
      list_remove(TLB, 0);
    }
    list_add(TLB, entrada);

    // punteroFifo = (punteroFifo + 1) % TLB_MAX_SIZE;

    // log_info(logger, "puntero despues de reemplazo: %d", punteroFifo);
    
    //mostrarTLB();
}

void mostrarTLB() {
    log_info(logger, "TLB::");
    for(int i=0; i<list_size(TLB);i++){
        tlb_entry* data = list_get(TLB, i);
        log_info(logger, "pag %d marco %d", data->pagina, data->marco);
    } 
}

int encontrarPagEnTLB(int PID, int numeroPagina) {
    for (int i = 0; i<list_size(TLB);i++) {
        tlb_entry* data = list_get(TLB,i);
        if (data->pid == PID && data->pagina == numeroPagina) {
            return i;
        }
    }
    return -1;
}

bool LRUYaPedido(int pid, int pagina) {
    bool encontrado = false;
    for (int i = 0; i < list_size(lru_peticiones) && !encontrado; i++) {
        tlb_entry* entrada = list_get(lru_peticiones, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            encontrado = true;
        }
    }
    return encontrado;
}

void agregarTimestamp(int pid, int pagina) {
    tlb_entry_lru* entrada = malloc(sizeof(tlb_entry_lru));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->timestamp = time(NULL);
    list_add(lru_peticiones, entrada);
}

void actualizarTimestamp(int pid, int pagina) {
    for (int i = 0; i < list_size(lru_peticiones); i++) {
        tlb_entry_lru* entrada = list_get(lru_peticiones, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            entrada->timestamp = time(NULL);
            break;
        }
    }
}

void actualizarTimestampOAgregarAPeticiones(int pid, int pagina) {
    if (LRUYaPedido(pid, pagina)) {
        actualizarTimestamp(pid, pagina);
    } else {
        agregarTimestamp(pid, pagina);
    }
}

bool estaEnTLB(int pid, int pagina) {
    bool encontrado = false;
    for (int i = 0; i < list_size(TLB) && !encontrado; i++) {
        tlb_entry* entrada = list_get(TLB, i);
        if (entrada->pid == pid && entrada->pagina == pagina) {
            encontrado = true;
        }
    }
    return encontrado;
}

// lru: Reemplaza la página cuya última referencia es la más lejana
void TLBagregarLRU(int pid, int pagina, int marco) {
    tlb_entry* entrada = malloc(sizeof(tlb_entry));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;

    list_sort(lru_peticiones, compareTimestamp);

    tlb_entry_lru* entrada_a_reemplazar = NULL;
    for (int i = 0; i < list_size(lru_peticiones); i++) {
        tlb_entry_lru* entradaBusqueda = list_get(lru_peticiones, i);
        if (estaEnTLB(entradaBusqueda->pid, entradaBusqueda->pagina)) {
            entrada_a_reemplazar = entradaBusqueda;
            break;
        }
    }

    for (int i = 0; i < list_size(TLB); i++) {
        tlb_entry* entradaTLB = list_get(TLB, i);
        if (entradaTLB->pid == entrada_a_reemplazar->pid && entradaTLB->pagina == entrada_a_reemplazar->pagina) {
            list_remove(TLB, i);
            list_add(TLB, entrada);
            break;
        }
    }

    list_remove(lru_peticiones, 0);
}