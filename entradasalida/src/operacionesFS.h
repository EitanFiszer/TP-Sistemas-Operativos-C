#ifndef operacionesFS_H
#define operacionesFS_H

void crear_archivo(const char* nombre, int block_count, int block_size);
void delete_archivo(const char* nombre, int block_count, int block_size);
void truncate_archivo(const char* nombre, int block_count, int block_size, int tam);

#endif