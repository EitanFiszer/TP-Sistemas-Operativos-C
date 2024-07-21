#ifndef INICIAR_H
#define INICIAR_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <commons/config.h>


t_log *iniciar_logger(char*, char*);
t_log *iniciar_logger_kernel(char *, char *);
t_config *iniciar_config(char *);
void terminar_programa(t_log*, t_config*);


#endif /* INICIAR_H */