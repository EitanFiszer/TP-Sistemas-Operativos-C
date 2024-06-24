#include <manejoInterfaces.h>
#include <stdlib.h>

t_dictionary* interfaces_dict;

typedef struct {
	char* nombre;
	char* tipo_interfaz;
    int socket_interfaz;
    bool conectada;
} datos_interfaz;

// Inicializa el diccionario en el punto de entrada principal de tu aplicación
void inicializar_interfaces() {
    interfaces_dict = dictionary_create();
}

void agregar_interfaz(char* nombre, char* tipo, int socket){
    datos_interfaz* nueva_interfaz = malloc(sizeof(datos_interfaz));
    nueva_interfaz->nombre = nombre;
    nueva_interfaz->tipo_interfaz = tipo;
    nueva_interfaz->socket_interfaz = socket;
    nueva_interfaz->conectada = true;
    dictionary_put(interfaces_dict, nombre, nueva_interfaz);
}
