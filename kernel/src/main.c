#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    // decir_hola("Kernel");

    t_log* logger = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("kernel.config");

    int puerto = config_get_int_value(config, "PUERTO_ESCUCHA");

    char* stringParaLogger = string_from_format("Hola, soy un kernel! Puerto: %d", puerto);
    log_info(logger, stringParaLogger);

    return 0;
}