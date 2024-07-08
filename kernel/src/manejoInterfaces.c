#include <manejoInterfaces.h>
#include <stdlib.h>

t_dictionary* interfaces_dict;

typedef struct {
	char* tipo_interfaz;
    int socket_interfaz;
    pthread_mutex_t* mutex_interfaz;
    t_queue* cola_blocked_interfaz;
    bool libre;
} datos_interfaz;

// Inicializa el diccionario en el punto de entrada principal de tu aplicación
void inicializar_interfaces() {
    interfaces_dict = dictionary_create();
}

void agregar_interfaz(char* nombre, char* tipo, int socket){
    datos_interfaz* nueva_interfaz = malloc(sizeof(datos_interfaz));
    nueva_interfaz->tipo_interfaz = tipo;
    nueva_interfaz->socket_interfaz = socket;
    nueva_interfaz->libre = true;
    nueva_interfaz->mutex_interfaz = malloc(sizeof(pthread_mutex_t));
    nueva_interfaz->cola_blocked_interfaz = queue_create();
    dictionary_put(interfaces_dict, nombre, nueva_interfaz);
}


void atender_io_stdin_read(t_payload_io_stdin_read* stdint_read){
    datos_interfaz* find_io = dictionary_get(interfaces_dict,stdint_read->interfaz);
    if(find_io == NULL){
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s" ,stdint_read->interfaz);
        lts_ex(stdint_read->pcb, EXEC);
    }else{
        pthread_mutex_lock(find_io->mutex_interfaz);
        if(!find_io->libre){
            log_info(logger, "INTERFAZ: %s ocupada, bloqueando proceso: %d",stdint_read->interfaz, stdint_read->pcb->PID);
            queue_push(find_io->cola_blocked_interfaz,stdint_read->pcb);
            log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", stdint_read->pcb->PID);
        }else{
            t_payload_io_stdin_read_de_kernel_a_io* payload = malloc(t_payload_io_stdin_read_de_kernel_a_io);
            int size_payload;
            
            void* buffer = serializar_stdin_read_de_kernel_a_io(payload,&size_payload);
            enviar_paquete_entre(find_io->socket_interfaz, IO_STDIN_READ, buffer, size_payload);

        }

        pthread_mutex_unlock(find_io->mutex_interfaz);
    }
}

void desconectar_IO(char* nombre_io_hilo){
    dictionary_remove_and_destroy(interfaces_dict,nombre_io_hilo);
}