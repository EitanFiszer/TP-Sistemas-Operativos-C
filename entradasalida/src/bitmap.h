#ifndef BITMAP_H
#define BITMAP_H

#include <commons/bitarray.h>

typedef struct {
    char* path;      // Ruta al archivo del bitmap
    t_bitarray* bits;       // Array de enteros para representar el bitmap
    int block_count; // Número total de bloques
} t_bitmap;

t_bitmap* crear_bitmap(int block_count, const char* path);
void marcar_bloque(t_bitmap* bitmap, int bloque);
void desmarcar_bloque(t_bitmap* bitmap, int bloque);
int obtener_bloque_libre(t_bitmap* bitmap);

#endif