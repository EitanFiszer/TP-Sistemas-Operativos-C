#ifndef TLB_H
#define TLB_H

#include <commons/collections/list.h>
#include <time.h>

typedef struct {
    int pid;     // identificador de proceso
    int pagina; // número de página
    int marco; // dirección física, -1 si no está en memoria
    time_t timestamp; // timestamp de creación
} tlb_entry;

typedef enum {
    FIFO,
    LRU
} tlb_reemplazo;

int marcoSegunPIDyPagina(int pid, int pagina);
bool tlbLlena(int TLB_SIZE);
void TLBagregarFIFO(int pid, int pagina, int marco);
void TLBagregarLRU(int pid, int pagina, int marco);

#endif // TLB_H