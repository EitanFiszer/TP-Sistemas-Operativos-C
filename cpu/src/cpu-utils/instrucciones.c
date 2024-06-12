#include "./instrucciones.h"
#include "./conexiones.h"
#include "./mmu.h"
#include <utils/constants.h>
#include <stdlib.h>
#include <commons/string.h>
#include <utils/client.h>

extern int socketKernel;

int valorDelRegistro(char* reg, registros_t* registros) {
    if(string_equals_ignore_case(reg, "AX")){
        return registros->AX;
    } else if (string_equals_ignore_case(reg, "BX")) {
        return registros->BX;
    } else if (string_equals_ignore_case(reg, "CX")) {
        return registros->CX;
    } else if (string_equals_ignore_case(reg, "DX")) {
        return registros->DX;
    } else if (string_equals_ignore_case(reg, "EAX")) {
        return registros->EAX;
    } else if (string_equals_ignore_case(reg, "EBX")) {
        return registros->EBX;
    } else if (string_equals_ignore_case(reg, "ECX")) {
        return registros->ECX;
    } else if (string_equals_ignore_case(reg, "EDX")) {
        return registros->EDX;
    } else if (string_equals_ignore_case(reg, "SI")) {
        return registros->SI;
    } else if (string_equals_ignore_case(reg, "DI")) {
        return registros->DI;
    }
    return 0;
}

// Asigna al registro el valor pasado como parámetro
void instruccionSet(char* reg, int valor, registros_t* registros) {
    if(string_equals_ignore_case(reg, "AX")){
        registros->AX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "BX")) {
        registros->BX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "CX")) {
        registros->CX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "DX")) {
        registros->DX = (uint8_t)valor;
    } else if (string_equals_ignore_case(reg, "EAX")) {
        registros->EAX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "EBX")) {
        registros->EBX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "ECX")) {
        registros->ECX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "EDX")) {
        registros->EDX = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "SI")) {
        registros->SI = (uint32_t)valor;
    } else if (string_equals_ignore_case(reg, "DI")) {
        registros->DI = (uint32_t)valor;
    }
}

// Suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino
void instruccionSum(t_PCB* pcb, char* regDest, char* regOrig, registros_t registros) {
    int valor1 = 0;
    int valor2 = 0;

    valor1 = valorDelRegistro(regDest, &registros);
    valor2 = valorDelRegistro(regOrig, &registros);

    instruccionSet(regDest, valor1 + valor2, &registros);
    pcb->program_counter = pcb->program_counter + 1;
}

// Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino
void instruccionSub(t_PCB* pcb, char* regDest, char* regOrig, registros_t registros) {
    int valor1 = 0;
    int valor2 = 0;

    valor1 = valorDelRegistro(regDest, &registros);
    valor2 = valorDelRegistro(regOrig, &registros);

    instruccionSet(regDest, valor1 - valor2, &registros);
    pcb->program_counter = pcb->program_counter + 1;
}

// Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro
void instruccionJNZ(t_PCB* pcb, char* reg, int instruccionASaltar, registros_t registros) {
    int valor = valorDelRegistro(reg, &registros);

    if(valor != 0) {
        pcb->program_counter = instruccionASaltar;
    } else {
        pcb->program_counter = pcb->program_counter + 1;
    }
}

// Esta instrucción solicita al Kernel que se envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo
void instruccionIoGenSleep(t_PCB* pcb, char* interfaz, int tiempo) {
    t_paquete* paq = crear_paquete();
    t_paquete_entre *paquete = malloc(sizeof(t_paquete_entre));
    paquete->operacion = SYSCALL;
    
    pcb->program_counter = pcb->program_counter + 1;

    t_payload_io_gen_sleep* nuestroPayload = malloc(sizeof(t_payload_io_gen_sleep));
    nuestroPayload->tiempo = tiempo;
    nuestroPayload->interfaz = interfaz; 
    nuestroPayload->instruccion = IO_GEN_SLEEP;
    nuestroPayload->pcb = pcb;
    
    paquete->payload = nuestroPayload;
    
    agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));

    enviar_paquete(paq, socketKernel);

    free(paq);
    pcb->program_counter = pcb->program_counter + 1;
}

// Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.
void instruccionMovIn(char* regDato, char* regDire, registros_t* registros, t_PCB* pcb) {
    int dirLogica = valorDelRegistro(regDire, registros);
    int dirFisica = calcularDireccionFisica(pcb->PID, dirLogica);

    int* dato = (int*)solicitar_dato_memoria(dirFisica);

    if (dato == NULL) {
        return;
    }

    instruccionSet(regDato, *dato, registros);
    pcb->program_counter = pcb->program_counter + 1;
}

// Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
void instruccionMovOut(char* regDire, char* regDato, registros_t* registros, t_PCB* pcb) {
    int dato = valorDelRegistro(regDato, registros);
    int dirLogica = valorDelRegistro(regDire, registros);

    int dirFisica = calcularDireccionFisica(pcb->PID, dirLogica);
    if (dirFisica == -1) {
        return;
    }

    enviar_dato_memoria(dirFisica, &dato);
    pcb->program_counter = pcb->program_counter + 1;
}

// Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
void instruccionResize(int tam, t_PCB* pcb) {
    int ok = solicitar_resize_memoria(pcb->PID, tam);

    if (ok == -1) {
        enviar_pcb_kernel(pcb, ERROR_OUT_OF_MEMORY);
    } else {    
        pcb->program_counter = pcb->program_counter + 1;
    }
}

// Toma del string apuntado por el registro SI y copia la cantidad de bytes indicadas en el parámetro tamaño a la posición de memoria apuntada por el registro DI.
void instruccionCopyString(int tam, registros_t registros, t_PCB* pcb) {
    int dirLogicaSI = valorDelRegistro("SI", &registros);
    int dirLogicaDI = valorDelRegistro("DI", &registros);

    int dirFisicaSI = calcularDireccionFisica(pcb->PID, dirLogicaSI);
    int dirFisicaDI = calcularDireccionFisica(pcb->PID, dirLogicaDI);

    char* string = (char*)solicitar_dato_memoria(dirFisicaSI);

    if (string == NULL) {
        return;
    }

    char* stringCortada = malloc(tam);
    strncpy(stringCortada, string, tam);

    int ok = enviar_dato_memoria(dirFisicaDI, stringCortada);

    if (ok == -1) {
        return;
    } else {
        pcb->program_counter = pcb->program_counter + 1;
    }
}

// Esta instrucción solicita al Kernel que se asigne una instancia del recurso indicado por parámetro.
void instruccionWait(char* recurso, t_PCB* pcb) {
    solicitar_wait(recurso);
    pcb->program_counter = pcb->program_counter +1;

}

// Esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro.
void instruccionSignal(char* recurso, t_PCB* pcb) {
    solicitar_signal(recurso);
    pcb->program_counter = pcb->program_counter + 1;
}

// Esta instrucción solicita al Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado) un valor cuyo tamaño está delimitado por el valor del Registro Tamaño y el mismo se guarde a partir de la Dirección Lógica almacenada en el Registro Dirección.
void instruccionIoSTDInRead(char* interfaz, char* regDire, char* regTam, registros_t* registros, t_PCB* pcb) {
    int dirLogica = valorDelRegistro(regDire, registros);
    int tam = valorDelRegistro(regTam, registros);

    char* string = solicitar_io_stdin(tam);

    if (string == NULL) {
        return;
    }

    int dirFisica = calcularDireccionFisica(pcb->PID, dirLogica);

    int ok = enviar_dato_memoria(dirFisica, string);

    if (ok == -1) {
        return;
    } else {
        pcb->program_counter = pcb->program_counter + 1;
    }
}

/*Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde la posición de memoria 
indicada por la Dirección Lógica almacenada en el Registro Dirección, un tamaño indicado por el Registro Tamaño 
y se imprima por pantalla.*/
void instruccionIoSTDOutWrite(char* interfaz, char* regDire, char* regTam, registros_t* registros, t_PCB* pcb) {
    solicitar_io_stdout(interfaz, regDire, regTam);
    pcb->program_counter = pcb->program_counter + 1;
}
//Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se cree un archivo en el FS montado en dicha interfaz.
void instruccionIoFSCreate(char* interfaz, char* nombreArchivo, t_PCB* pcb) {
    solicitar_fs_createORdelete(interfaz, nombreArchivo, FS_CREATE);
    pcb->program_counter = pcb->program_counter + 1;
}
//Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, 
//se elimine un archivo en el FS montado en dicha interfaz
void instruccionIoFSDelete(char* interfaz, char* nombreArchivo, t_PCB* pcb) {
    solicitar_fs_createORdelete(interfaz, nombreArchivo, FS_DELETE);
    pcb->program_counter = pcb->program_counter + 1;
}

/*Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se modifique el tamaño del archivo en 
el FS montado en dicha interfaz, actualizando al valor que se encuentra en el registro indicado por Registro Tamaño.*/
void instruccionIoFSTruncate(char* interfaz, char* nombreArchivo, char* regTam, registros_t* registros, t_PCB* pcb) {
    solicitar_fs_truncate(interfaz, nombreArchivo, regTam);
    pcb->program_counter = pcb->program_counter + 1;
}

/*Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde Memoria la cantidad de 
bytes indicadas por el Registro Tamaño a partir de la dirección lógica que se encuentra en el Registro Dirección y 
se escriban en el archivo a partir del valor del Registro Puntero Archivo.*/
void instruccionIoFSWrite(char* interfaz, char* nombreArchivo, char* regDire, char* regTam, char* regPuntero, registros_t* registros, t_PCB* pcb) {
    solicitar_fs_writeORread(interfaz, nombreArchivo, regDire, regTam, regPuntero, FS_WRITE);
    pcb->program_counter = pcb->program_counter + 1;
}

/*Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde el archivo a partir del 
valor del Registro Puntero Archivo la cantidad de bytes indicada por Registro Tamaño y se escriban en la Memoria a 
partir de la dirección lógica indicada en el Registro Dirección*/
void instruccionIoFSRead(char* interfaz, char* nombreArchivo, char* regDire, char* regTam, char* regPuntero, registros_t* registros, t_PCB* pcb) {
    solicitar_fs_writeORread(interfaz, nombreArchivo, regDire, regTam, regPuntero, FS_READ);
    pcb->program_counter = pcb->program_counter + 1;
}

/*Esta instrucción representa la syscall de finalización del proceso. Se deberá devolver el 
Contexto de Ejecución actualizado al Kernel para su finalización.*/
void instruccionExit(t_PCB* pcb) {
    enviar_pcb_kernel(pcb, EXIT_PROCESS);
}