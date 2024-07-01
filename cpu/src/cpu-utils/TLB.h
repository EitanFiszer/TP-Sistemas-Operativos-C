#ifndef TLB_H
#define TLB_H

#include <commons/collections/list.h>
#include <time.h>

typedef struct {
    int pid;     // identificador de proceso
    int pagina;  // número de página
    int marco;   // dirección física, -1 si no está en memoria
} tlb_entry;

typedef struct {
    int pid;
    int pagina;
    time_t timestamp;
} tlb_entry_lru;

typedef enum {
    FIFO,
    LRU
} tlb_reemplazo;

int marcoSegunPIDyPagina(int pid, int pagina);
bool tlbLlena();
void TLBagregarFIFO(int pid, int pagina, int marco);
void TLBagregarLRU(int pid, int pagina, int marco);
void actualizarTimestampOAgregarAPeticiones(int pid, int pagina);
void agregarEntradaTLB(int pid, int pagina, int marco);

#endif  // TLB_H