# Instrucciones
- [x] Devolver a CPU la instruccion indicada por el Program Counter y su PID
  - [x] Esperar tiempo determinado a modo de retardo, puesto en el .config
- [x] Creación del proceso
  - [x] Generar estructuras necesarias para poder devolver de a 1 las instrucciones

# Esquema de memoria y estructuras
## Paginación simple
- [ ] Un espacio de memoria (representado por un void*). Este represetará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir
- [ ] Tabla de páginas

Es importante aclarar que cualquier implementación que no tenga todo el espacio de memoria dedicado a representar el espacio de usuario de manera contigua será motivo de desaprobación directa, para esto se puede llegar a controlar la implementación a la hora de iniciar la evaluación.
El tamaño de la memoria siempre será un múltiplo del tamaño de página.

# Comunicación con Kernel, CPU e Interfaces de I/O

## Creación de proceso
- [x] Viene del kernel, se le pasa archivo de texto con instrucciones

## Finalización de proceso
- [x] Viene del kernel, debe liberar el espacio de memoria. Marcar los frames como libres, no sobreescribirlos.

## Acceso a tabla de páginas
- [ ]  Deberá responder el número de marco correspondiente a la página consultada

## Ajustar tamaño de un proceso
Hay 2 opciones que pueden pasar cuando llega una instrucción de resize:
### Ampliación de un proceso
Se deberá ampliar el tamaño del proceso al final del mismo, pudiendo solicitarse múltples páginas. Es posible que en un punto no se puedan solicitar más marcos ya que la memoria se encuentra llena, en este caso se deberá devolver un error OUT_OF_MEMORY.

### Reducción de un proceso
Se reducirá el mismo desde el final, liberando, en caso de ser necesario, las páginas que ya no se utilicen (desde la última hacia la primera).

## Acceso a espacio de usuario
Esta petición puede venir tanto de la CPU como de un módulo de I/O. Es importante que las peticiones pueden ocupar más de una página.

## Retardo en peticiones
Ante cada una de las peticiones definidas anteriormente se deberá aplicar el tiempo de espera en milisegundos definido en el archivo de configuración.

## Logs mínimos y obligatorios
Creación / destrucción de Tabla de Páginas: “PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>”

Acceso a Tabla de Páginas: “PID: <PID> - Pagina: <PAGINA> - Marco: <MARCO>” 

Ampliación de Proceso: “PID: <PID> - Tamaño Actual: <TAMAÑO_ACTUAL> - Tamaño a Ampliar: <TAMAÑO_A_AMPLIAR>” 

Reducción de Proceso: “PID: <PID> - Tamaño Actual: <TAMAÑO_ACTUAL> - Tamaño a Reducir: <TAMAÑO_A_REDUCIR>” 

Acceso a espacio de usuario: “PID: <PID> - Accion: <LEER / ESCRIBIR> - Direccion fisica: <DIRECCION_FISICA>” - Tamaño <TAMAÑO A LEER / ESCRIBIR>
