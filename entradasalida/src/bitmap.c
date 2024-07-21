#include "bitmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <commons/bitarray.h>

t_bitmap* crear_bitmap(int block_count, const char* path) {
    t_bitmap* bitmap = malloc(sizeof(t_bitmap));
    
    t_bitarray* bits = bitarray_create_with_mode(NULL, block_count, MSB_FIRST);
    bitmap->bits =  bits // Inicializa todos los bits a 0
    bitmap->block_count = block_count;
    bitmap->path = strdup(path);
    return bitmap;
}

int obtener_bloque_libre(t_bitarray* bitmap) {
    t_bitarray* bits = bitmap->bits  
    int max = bitarray_get_max_bit(bits);

    for (int i = 0; i < max; i++) {
        if (!bitarray_test_bit(bits, i)) {
            return i;
        }
    }
    return -1;
}

void marcar_bloque(t_bitmap* bitmap, int bloque){
    int max = bitarray_get_max_bit(bitmap->bits);
    if (bloque >= 0 && bloque < max){
        bitarray_set_bit(bitmap->bits, bloque)
    }
}


void desmarcar_bloque(t_bitmap* bitmap, int bloque){
    int max = bitarray_get_max_bit(bitmap->bits);
    if (bloque >= 0 && bloque < max){
        bitarray_clean_bit(bitmap->bits, bloque)
    }
}