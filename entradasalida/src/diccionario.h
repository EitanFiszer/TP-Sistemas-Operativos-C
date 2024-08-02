#include <stdio.h> 
#include <dirent.h>
#include <commons/collections/dictionary.h>
#include <utils/constants.h>

void leerDiccionario();
t_dictionary* incializar_el_diccionario();
int espacioLIbre(char* nombre, int cant_bloques_arch);
void cargar_diccionario_nuevo(char* nombre, int bloque_inicial);
t_diccionario* getFCBxInicio(int bloque_inicio);

