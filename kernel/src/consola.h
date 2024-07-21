#ifndef CONSOLA_H
#define CONSOLA_H

#include "global.h"
#include "planificacion.h"
#include <readline/readline.h>
#include <commons/string.h>

void* consola_interactiva();
int ejecutar_script(char* path);
#endif // CONSOLA_H
