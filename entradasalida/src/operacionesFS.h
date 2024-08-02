#ifndef operacionesFS_H
#define operacionesFS_H

void crearArchivodebloques();
void inicializar_FS();
void crear_archivo(char* nombre);
void delete_archivo( char* nombre);
void truncate_archivo( char* nombre, int tam, int retraso_compactacion);
void compactacion_metadata (char* nombre, int espacio);
void escribir_archivo(char* nombre, int puntero, int tam, void* dato);
void* leer_archivo(char* nombre, int puntero, int tam);

#endif