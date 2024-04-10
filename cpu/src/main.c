#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

int main(int argc, char* argv[]) {
    // decir_hola("CPU");

    t_log* logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
    t_config* config = config_create("cpu.config");

    int puerto = config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");

    char* stringParaLogger = string_from_format("Hola, soy un cpu! Puerto: %d", puerto);
    log_info(logger, stringParaLogger);

    return 0;
}
