#include "TLB.h"
#include <commons/string.h>
#include <commons/log.h>
#include <stdlib.h>

extern tlb_reemplazo TLB_ALGORITMO_REEMPLAZO;
extern int TLB_MAX_SIZE;
extern t_list* TLB;
extern t_log* logger;

int punteroFifo = 0;
t_list* lru_peticiones;

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
        if (string_equals_ignore_case((char*)TLB_ALGORITMO_REEMPLAZO, "FIFO")) {
            TLBagregarFIFO(pid, pagina, marco);
        } else if (string_equals_ignore_case((char*)TLB_ALGORITMO_REEMPLAZO, "LRU")) {
            TLBagregarLRU(pid, pagina, marco);
        } else {
            // no debería llegar acá
            log_error(logger, "Algoritmo de reemplazo de TLB no válido");
            return;
        }
    }

    tlb_entry* entrada = malloc(sizeof(tlb_entry));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;
    list_add(TLB, entrada);
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
    
    list_remove(TLB, punteroFifo);
    list_add(TLB, entrada);

    punteroFifo = (punteroFifo + 1) % TLB_MAX_SIZE;
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

// lru: Reemplaza la página cuya última referencia es la más lejana
void TLBagregarLRU(int pid, int pagina, int marco) {
    tlb_entry* entrada = malloc(sizeof(tlb_entry));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;


    if(lru_peticiones == NULL) {
        lru_peticiones = list_create();
    }

    if(LRUYaPedido(pid, pagina)) {
        actualizarTimestamp(pid, pagina);
    } else {
        agregarTimestamp(pid, pagina);
    }

    list_sort(lru_peticiones, compareTimestamp);

    tlb_entry* entrada_a_reemplazar = list_get(lru_peticiones, 0);
    for (int i = 0; i < list_size(TLB); i++) {
        tlb_entry* entrada = list_get(TLB, i);
        if (entrada->pid == entrada_a_reemplazar->pid && entrada->pagina == entrada_a_reemplazar->pagina) {
            list_remove(TLB, i);
            list_add(TLB, entrada);
            break;
        }
    }
    list_remove(lru_peticiones, 0);
}