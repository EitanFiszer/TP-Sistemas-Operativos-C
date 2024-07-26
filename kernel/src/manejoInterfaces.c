#include <manejoInterfaces.h>
#include <stdlib.h>

t_dictionary *interfaces_dict;
t_dictionary *operaciones_espera_dict;

char* int_to_string(int value) {
    int length = snprintf(NULL, 0, "%d", value);
    char *str = malloc(length + 1);
    sprintf(str, "%d", value);
    return str;
}
typedef struct
{
    char *tipo_interfaz;
    int socket_interfaz;
    pthread_mutex_t mutex_interfaz;
    t_queue *cola_blocked_interfaz;
    t_PCB *pcb_usando_interfaz;
    bool libre;
    
} datos_interfaz;

typedef struct
{
    int size_payload;
    void *buffer;
    OP_CODES_ENTRE op;
} dato_op_espera;

void inicializar_operaciones_en_espera()
{
    operaciones_espera_dict = dictionary_create();
}

// Inicializa el diccionario en el punto de entrada principal de tu aplicación
void inicializar_interfaces()
{
    interfaces_dict = dictionary_create();
}

void agregar_interfaz(char *nombre, char *tipo, int socket)
{
    datos_interfaz *nueva_interfaz = malloc(sizeof(datos_interfaz));
    nueva_interfaz->tipo_interfaz = tipo;
    nueva_interfaz->socket_interfaz = socket;
    nueva_interfaz->libre = true;
    pthread_mutex_init(&(nueva_interfaz->mutex_interfaz), NULL);

        // nueva_interfaz->mutex_interfaz = malloc(sizeof(pthread_mutex_t));
    nueva_interfaz->cola_blocked_interfaz = queue_create();
    nueva_interfaz->pcb_usando_interfaz = malloc(sizeof(t_PCB));
    dictionary_put(interfaces_dict, nombre, nueva_interfaz);
}

void atender_io_stdin_read(t_payload_io_stdin_read *stdint_read)
{
    datos_interfaz *find_io = dictionary_get(interfaces_dict, stdint_read->interfaz);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", stdint_read->interfaz);
        lts_ex(stdint_read->pcb, EXEC,"INVALID_INTERFACE");
    }
    else
    {
        pthread_mutex_lock(&(find_io->mutex_interfaz));
        if (strcmp(find_io->tipo_interfaz, "IO_STDIN") == 0)
        {
            add_queue_blocked(stdint_read->pcb,IOB,stdint_read->interfaz);

            t_payload_io_stdin_read_de_kernel_a_io *payload = malloc(sizeof(t_payload_io_stdin_read_de_kernel_a_io));

            int size_payload;

            payload->direccionFisica = stdint_read->dirFisica;
            payload->interfaz = stdint_read->interfaz;
            payload->tam = stdint_read->tam;

            void *buffer = serializar_stdin_read_de_kernel_a_io(payload, &size_payload);

            if (!find_io->libre)
            {
                dato_op_espera *espera = malloc(sizeof(dato_op_espera));
                espera->size_payload = size_payload;
                espera->buffer = buffer;
                espera->op = IO_STDIN_READ;
                dictionary_put(operaciones_espera_dict, int_to_string(stdint_read->pcb->PID), espera);
                log_info(logger, "INTERFAZ: %s ocupada, proceso: %d en espera de interfaz", stdint_read->interfaz, stdint_read->pcb->PID);
                queue_push(find_io->cola_blocked_interfaz, stdint_read->pcb);
                log_info(logger,"PID: %d - Bloqueado por: <%s>",stdint_read->pcb->PID,stdint_read->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", stdint_read->pcb->PID);
            }
            else
            {
                enviar_paquete_entre(find_io->socket_interfaz, IO_STDIN_READ, buffer, size_payload);
                find_io->pcb_usando_interfaz = stdint_read->pcb;

                log_info(logger, "PID:%d -Estado Anterior:EXEC-Estado Actual:BLOCKED", stdint_read->pcb->PID);
            }
        }
        else
        {
            log_info(logger, "LA INTERFAZ <%s> NO ACEPTA LA OPERACION: IO_STDIN_READ", stdint_read->interfaz);
            lts_ex(stdint_read->pcb, EXEC, "INVALID_INTERFACE");
        }

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}
void atender_io_stdout_write(t_payload_io_stdout_write *stdout_write)
{
    datos_interfaz *find_io = dictionary_get(interfaces_dict, stdout_write->interfaz);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", stdout_write->interfaz);
        lts_ex(stdout_write->pcb, EXEC,"INVALID_INTERFACE");
    }
    else
    {
        if (strcmp(find_io->tipo_interfaz, "IO_STDOUT_WRITE") == 0)
        {
            add_queue_blocked(stdout_write->pcb, IOB, stdout_write->interfaz);

            pthread_mutex_lock(&(find_io->mutex_interfaz));
            int size_payload;
            void *buffer = serializar_io_stdout_write(stdout_write, &size_payload);
            if (!find_io->libre)
            {
                dato_op_espera *espera = malloc(sizeof(dato_op_espera));
                espera->size_payload = size_payload;
                espera->buffer = buffer;
                espera->op = IO_STDOUT_WRITE;
                dictionary_put(operaciones_espera_dict, int_to_string(stdout_write->pcb->PID), espera);
                log_info(logger, "INTERFAZ: %s ocupada, proceso: %d en espera de interfaz", stdout_write->interfaz, stdout_write->pcb->PID);
                queue_push(find_io->cola_blocked_interfaz, stdout_write->pcb);
                log_info(logger,"PID: %d - Bloqueado por: <%s>",stdout_write->pcb->PID,stdout_write->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", stdout_write->pcb->PID);
            }
            else
            {
                enviar_paquete_entre(find_io->socket_interfaz, IO_STDOUT_WRITE, buffer, size_payload);
                find_io->pcb_usando_interfaz = stdout_write->pcb;
                log_info(logger,"PID: %d - Bloqueado por: <%s>",stdout_write->pcb->PID,stdout_write->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", find_io->pcb_usando_interfaz->PID);
            }
        }
        else
        {
            log_info(logger, "LA INTERFAZ <%s> NO ACEPTA LA OPERACION: IO_STDOUT_WRITE", stdout_write->interfaz);
            lts_ex(stdout_write->pcb, EXEC,"INVALID_INTERFACE");
        }

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}
void atender_fs_createOrDelate(t_payload_fs_create *createOrDelate, OP_CODES_ENTRE code)
{
    datos_interfaz *find_io = dictionary_get(interfaces_dict, createOrDelate->interfaz);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", createOrDelate->interfaz);
        lts_ex(createOrDelate->pcb, EXEC,"INVALID_INTERFACE");
    }
    else
    {
        pthread_mutex_lock(&(find_io->mutex_interfaz));
        if (strcmp(find_io->tipo_interfaz, "DIALFS") == 0)
        { // VER QUE RECIBE IO
            add_queue_blocked(createOrDelate->pcb,IOB,createOrDelate->interfaz);
            int size_payload;
            void *buffer = serializar_fs_create(createOrDelate, &size_payload);
            if (!find_io->libre)
            {
                dato_op_espera *espera = malloc(sizeof(dato_op_espera));
                espera->size_payload = size_payload;
                espera->buffer = buffer;
                espera->op = code;
                dictionary_put(operaciones_espera_dict, int_to_string(createOrDelate->pcb->PID), espera);
                log_info(logger, "INTERFAZ: %s ocupada, proceso: %d en espera de interfaz", createOrDelate->interfaz, createOrDelate->pcb->PID);
                queue_push(find_io->cola_blocked_interfaz, createOrDelate->pcb);
                log_info(logger,"PID: %d - Bloqueado por: <%s>",createOrDelate->pcb->PID,createOrDelate->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", createOrDelate->pcb->PID);
            }
            else
            {

                enviar_paquete_entre(find_io->socket_interfaz, code, buffer, size_payload);
                find_io->pcb_usando_interfaz = createOrDelate->pcb;
                log_info(logger,"PID: %d - Bloqueado por: <%s>",createOrDelate->pcb->PID,createOrDelate->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", find_io->pcb_usando_interfaz->PID);
            }
        }
        else
        {
            log_info(logger, "LA INTERFAZ <%s> NO ACEPTA LA OPERACION: FS_CREATE y FS_DELATE", createOrDelate->interfaz);
            lts_ex(createOrDelate->pcb, EXEC,"INVALID_INTERFACE");
        }

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}
void atender_fs_truncate(t_payload_fs_truncate *fs_truncate)
{
    datos_interfaz *find_io = dictionary_get(interfaces_dict, fs_truncate->interfaz);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", fs_truncate->interfaz);
        lts_ex(fs_truncate->pcb, EXEC,"INVALID_INTERFACE");
    }
    else
    {
        if (strcmp(find_io->tipo_interfaz, "DIALFS") == 0)
        {
            add_queue_blocked(fs_truncate->pcb, IOB, fs_truncate->interfaz);
            int size_payload;
            void *buffer = serializar_fs_truncate(fs_truncate, &size_payload);
            if (!find_io->libre)
            {
                dato_op_espera *espera = malloc(sizeof(dato_op_espera));
                espera->size_payload = size_payload;
                espera->buffer = buffer;
                espera->op = IO_FS_TRUNCATE;
                dictionary_put(operaciones_espera_dict, int_to_string(fs_truncate->pcb->PID), espera);
                log_info(logger, "INTERFAZ: %s ocupada, proceso: %d en espera de interfaz", fs_truncate->interfaz, fs_truncate->pcb->PID);
                queue_push(find_io->cola_blocked_interfaz, fs_truncate->pcb);
                log_info(logger,"PID: %d - Bloqueado por: <%s>",fs_truncate->pcb->PID,fs_truncate->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", fs_truncate->pcb->PID);
            }
            else
            {
                enviar_paquete_entre(find_io->socket_interfaz, IO_FS_TRUNCATE /*ver que recibe io*/, buffer, size_payload);
                find_io->pcb_usando_interfaz = fs_truncate->pcb;
                log_info(logger,"PID: %d - Bloqueado por: <%s>",fs_truncate->pcb->PID,fs_truncate->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", find_io->pcb_usando_interfaz->PID);
            }
        }
        else
        {
            log_info(logger, "LA INTERFAZ <%s> NO ACEPTA LA OPERACION: FS_TRUNCATE", fs_truncate->interfaz);
            lts_ex(fs_truncate->pcb, EXEC,"INVALID_INTERFACE");
        }
        pthread_mutex_lock(&(find_io->mutex_interfaz));

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}
void atender_fs_writeOrRead(t_payload_fs_writeORread *writeOrRead, OP_CODES_ENTRE code)
{
    datos_interfaz *find_io = dictionary_get(interfaces_dict, writeOrRead->interfaz);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", writeOrRead->interfaz);
        lts_ex(writeOrRead->pcb, EXEC,"INVALID_INTERFACE");
    }
    else
    {
        pthread_mutex_lock(&(find_io->mutex_interfaz));

        if (strcmp(find_io->tipo_interfaz, "DIALFS") == 0)
        {
            add_queue_blocked(writeOrRead->pcb, IOB, writeOrRead->interfaz);
            int size_payload;
            void *buffer = serializar_fs_writeORread(writeOrRead, &size_payload);
            if (!find_io->libre)
            {
                dato_op_espera *espera = malloc(sizeof(dato_op_espera));
                espera->size_payload = size_payload;
                espera->buffer = buffer;
                espera->op = code;
                dictionary_put(operaciones_espera_dict, int_to_string(writeOrRead->pcb->PID), espera);
                log_info(logger, "INTERFAZ: %s ocupada, proceso: %d en espera de interfaz", writeOrRead->interfaz, writeOrRead->pcb->PID);
                queue_push(find_io->cola_blocked_interfaz, writeOrRead->pcb);
                log_info(logger,"PID: %d - Bloqueado por: <%s>",writeOrRead->pcb->PID,writeOrRead->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", writeOrRead->pcb->PID);
            }
            else
            {
                // VER QUE RECIBE IO

                enviar_paquete_entre(find_io->socket_interfaz, code, buffer, size_payload);
                find_io->pcb_usando_interfaz = writeOrRead->pcb;
                log_info(logger,"PID: %d - Bloqueado por: <%s>",writeOrRead->pcb->PID,writeOrRead->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", find_io->pcb_usando_interfaz->PID);
            }
        }
        else
        {
            log_info(logger, "LA INTERFAZ <%s> NO ACEPTA LA OPERACION: FS_WRITE y FS_READ", writeOrRead->interfaz);
            lts_ex(writeOrRead->pcb, EXEC,"INVALID_INTERFACE");
        }

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}
void atender_io_gen_sleep(t_payload_io_gen_sleep *genSleep)
{
    datos_interfaz *find_io = dictionary_get(interfaces_dict, genSleep->interfaz);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", genSleep->interfaz);
        lts_ex(genSleep->pcb, EXEC,"INVALID_INTERFACE");
    }
    else
    {
        if (strcmp(find_io->tipo_interfaz, "GENERICA") == 0)
        {
            add_queue_blocked(genSleep->pcb, IOB, genSleep->interfaz);
            int size_payload;
            void *buffer = serializar_io_gen_sleep(genSleep, &size_payload);
            pthread_mutex_lock(&(find_io->mutex_interfaz));
            if (!find_io->libre)
            {
                dato_op_espera *espera = malloc(sizeof(dato_op_espera));
                espera->size_payload = size_payload;
                espera->buffer = buffer;
                espera->op = IO_GEN_SLEEP;
                dictionary_put(operaciones_espera_dict, int_to_string(genSleep->pcb->PID), espera);
                log_info(logger, "INTERFAZ: %s ocupada, proceso: %d en espera de interfaz", genSleep->interfaz, genSleep->pcb->PID);
                queue_push(find_io->cola_blocked_interfaz, genSleep->pcb);
                log_info(logger,"PID: %d - Bloqueado por: <%s>",genSleep->pcb->PID,genSleep->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", genSleep->pcb->PID);
            }
            else
            {

                // VER QUE RECIBE IO

                enviar_paquete_entre(find_io->socket_interfaz, IO_GEN_SLEEP, buffer, size_payload);
                find_io->pcb_usando_interfaz = genSleep->pcb;
                log_info(logger,"PID: %d - Bloqueado por: <%s>",genSleep->pcb->PID,genSleep->interfaz);
                log_info(logger, "PID:%d - Estado Anterior: EXEC - Estado Actual: BLOCKED", find_io->pcb_usando_interfaz->PID);
            }
        }
        else
        {
            log_info(logger, "LA INTERFAZ <%s> NO ACEPTA LA OPERACION: IO_GEN_SLEEP", genSleep->interfaz);
            lts_ex(genSleep->pcb, EXEC,"INVALID_INTERFACE");
        }

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}

void removerBuffer(void* element) {
    free(element);
}

void desconectar_IO(char *nombre_io_hilo)
{
    datos_interfaz *datos_io = dictionary_remove(interfaces_dict, nombre_io_hilo);
    if (datos_io != NULL)
    {
        pthread_mutex_lock(&(datos_io->mutex_interfaz));
        log_info(logger, "INTERFAZ: %s desconectada, enviando procesos bloqueados a exit", nombre_io_hilo);
        while (!queue_is_empty(datos_io->cola_blocked_interfaz))
        {
            t_PCB *retirada = queue_pop(datos_io->cola_blocked_interfaz);
            delete_queue_blocked(retirada);
            dictionary_remove_and_destroy(operaciones_espera_dict,int_to_string(retirada->PID),removerBuffer);
            lts_ex(retirada, BLOCKED,"INVALID_INTERFACE");
        }
        pthread_mutex_unlock(&(datos_io->mutex_interfaz));
    }
    free(datos_io);
}
void desocupar_io(char* nombre_io_hilo){
    datos_interfaz *find_io = dictionary_get(interfaces_dict, nombre_io_hilo);
    if (find_io == NULL)
    {
        log_info(logger, "NO EXITE INTERFAZ CON NOMBRE: %s", nombre_io_hilo);
    }
    else{
        pthread_mutex_lock(&(find_io->mutex_interfaz));
        delete_queue_blocked(find_io->pcb_usando_interfaz);
        cargar_ready(find_io->pcb_usando_interfaz,BLOCKED);

        find_io->pcb_usando_interfaz = NULL;

        if(!queue_is_empty(find_io->cola_blocked_interfaz)){
            t_PCB* prox = queue_pop(find_io->cola_blocked_interfaz);
            if(prox==NULL){
                log_info(logger,"HUBO UN ERROR AL REVISAR LOS PROCESOS BLOQUEADOS DE LA INTERFAZ: %s", nombre_io_hilo);
            }else{
                dato_op_espera* payload = dictionary_get(operaciones_espera_dict,int_to_string(prox->PID));
                enviar_paquete_entre(find_io->socket_interfaz,payload->op,payload->buffer,payload->size_payload);
                free(payload);
                find_io->pcb_usando_interfaz = prox;
            }
        }

        pthread_mutex_unlock(&(find_io->mutex_interfaz));
    }
}

void remove_cola_blocked_io(char *nombre_interfaz, t_PCB *pcb)
{
   datos_interfaz *find_io = dictionary_get(interfaces_dict, nombre_interfaz);
    if (find_io != NULL)
    {
        if(pcb->PID == find_io->pcb_usando_interfaz->PID){
            desocupar_io(nombre_interfaz);
        }else{
            pthread_mutex_lock(&(find_io->mutex_interfaz));

        t_PCB *pcb_aux = malloc(sizeof(t_PCB));
        int tam_cola_bloc = queue_size(find_io->cola_blocked_interfaz);
        for (int i = 0; i < tam_cola_bloc; i++)
        {
            pcb_aux = queue_pop(find_io->cola_blocked_interfaz);
            if (pcb_aux->PID != pcb->PID)
            {
                queue_push(find_io->cola_blocked_interfaz, pcb_aux);
            }
        }
            pthread_mutex_unlock(&(find_io->mutex_interfaz));
        }
    }
}
