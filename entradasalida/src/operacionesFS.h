#ifndef operacionesFS_H
#define operacionesFS_H

void crearArchivodebloques();
void inicializar_FS();
void crear_archivo(char* nombre);
void delete_archivo( char* nombre);
void truncate_archivo( char* nombre, int tam);

#endif