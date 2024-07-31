#ifndef BITMAP_H
#define BITMAP_H

#include <commons/bitarray.h>



void crear_bitmap();
t_bitarray* cargar_bitmap();
int getBit();
void setBitmap(int bloque);
void cleanBitMap(int bloque);
bool verificar_bitmap(int bloque, int cant_bloques);

#endif