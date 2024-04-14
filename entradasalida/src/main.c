#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    t_log* logger = log_create("entradasalida.log", "Entrada_Salida", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("entradasalida.config");

    int puerto = config_get_int_value(config, "PUERTO_MEMORIA");

    char* stringParaLogger = string_from_format("[EyS] Escuchando en el puerto: %d", puerto);
    log_info(logger, stringParaLogger);

    return 0;
}
