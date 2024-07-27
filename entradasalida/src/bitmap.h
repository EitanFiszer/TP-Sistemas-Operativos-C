#ifndef BITMAP_H
#define BITMAP_H

#include <commons/bitarray.h>



void crear_bitmap(int block_count);
int getBit(int block_count);
void setBitmap(int bloque, int block_count);
void cleanBitMap(int bloque, int block_count);
bool verificar_bitmap(int bloque, int cant_bloques, int block_count);

#endif