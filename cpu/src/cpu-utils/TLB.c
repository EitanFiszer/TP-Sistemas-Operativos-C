#include "TLB.h"
#include <commons/string.h>
#include <commons/log.h>
#include <stdlib.h>

extern tlb_reemplazo TLB_ALGORITMO_REEMPLAZO;
extern int TLB_MAX_SIZE;
extern t_list* TLB;
extern t_log* logger;

int punteroFifo = 0;

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

bool tlbLlena(int TLB_SIZE) {
    return list_size(TLB) == TLB_SIZE;
}

void agregarEntradaTLB(int pid, int pagina, int marco, int TLB_SIZE) {
    if (tlbLlena(TLB_SIZE)) {
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
    entrada->timestamp = time(NULL);
    list_add(TLB, entrada);
}

bool compareTimestamp(void* a, void* b) {
    return ((tlb_entry*)a)->timestamp < ((tlb_entry*)b)->timestamp;
}

// fifo: sacar el primero y agregar al final
void TLBagregarFIFO(int pid, int pagina, int marco) {
    tlb_entry* entrada = malloc(sizeof(tlb_entry));
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;
    entrada->timestamp = time(NULL);
    
    list_remove(TLB, punteroFifo);
    list_add(TLB, entrada);

    punteroFifo = (punteroFifo + 1) % TLB_MAX_SIZE;
}

// lru: Reemplaza la página cuya última referencia es la más lejana
void TLBagregarLRU(int pid, int pagina, int marco) {
    // LRU
}