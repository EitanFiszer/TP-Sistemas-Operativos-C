#ifndef BITMAP_H
#define BITMAP_H

#include <commons/bitarray.h>



void crear_bitmap(int block_count, const char* path);
int getBit(const char* path, int block_count);
void setBitmap(const char* path, int bloque, int block_count);
void cleanBitMap(const char* path, int bloque, int block_count);

#endif