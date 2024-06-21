#include "iniciar.h"

t_log *iniciar_logger(char *path, char *process_name)
{
    t_log *nuevo_logger = log_create(path, process_name, true, LOG_LEVEL_INFO);
    if (nuevo_logger == NULL)
    {
        perror("Hubo error a intentar inciar el logger");
        exit(EXIT_FAILURE);
    }
    return nuevo_logger;
}

t_config *iniciar_config(char *path)
{
    t_config *nuevo_config = config_create(path);
    if (nuevo_config == NULL)
    {
        perror("Hubo error a intentar inciar la configuracion");
        exit(EXIT_FAILURE);
    }
    return nuevo_config;
}

void terminar_programa(t_log *logger, t_config *config)
{
    log_destroy(logger);
    config_destroy(config);
}
