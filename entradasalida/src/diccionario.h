#include <stdio.h> 
#include <dirent.h>
#include <commons/collections/dictionary.h>


void leerDiccionario();
t_dictionary* incializar_el_diccionario();
int espacioLIbre(char* nombre, int cant_bloques_arch);
void cargar_diccionario_nuevo(char* nombre, int bloque_inicial);

