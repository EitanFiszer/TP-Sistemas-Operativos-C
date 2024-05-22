// #include <conexiones.h>

// //funciones de conexion
// void enviar_instrucciones_memoria(char* path)
// {
//     t_paquete *nuevo_paquete = crear_paquete();
//     t_paquete_entre *instruccion;
//     instruccion = malloc(sizeof(t_paquete_entre));
//     instruccion->operacion = CREAR_PROCESO;
//     payload_crear_proceso *payload = malloc(sizeof(payload_crear_proceso));
//     payload->path = path;
//     payload->pid = PID;
//     instruccion->payload = payload;
//     agregar_a_paquete(nuevo_paquete, instruccion, sizeof(t_paquete_entre));
//     // envio el paquete a la memoria //ENVIO EL NUEVO PROCESO
//     enviar_paquete(nuevo_paquete, resultHandshakeMemoria);

//     eliminar_paquete(nuevo_paquete);
// }
// void esperar_paquetes_cpu_dispatch(void)
// {
//     while (1)
//     {
//         t_list *paquete = recibir_paquete(resultHandshakeDispatch);
//         t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
//         switch (paquete_dispatch->operacion)
//         {
//         case DESALOJAR:
//             desalojar(paquete_dispatch->payload);
//             break;

//         default:
//             log_error(logger, "no se recibio paquete de la CPU, error");
//             break;
//         }
//     }
// }
// void esperar_paquetes_memoria(void)
// {
//     while (1)
//     {
//         t_list *paquete = recibir_paquete(resultHandshakeDispatch);
//         t_paquete_entre *paquete_dispatch = list_get(paquete, 0);
//         switch (paquete_dispatch->operacion)
//         {
//         case INSTRUCCIONES_CARGADAS:
//             // permite continuar con la planificacion a largo plazo
//             break;
//         default:
//             log_error(logger, "no se recibio paquete de la memoria, error");
//             break;
//         }
//     }
// }
// void enviar_paquete_cpu_dispatch (OP_CODES_ENTRE operacion,void* payload){
//     t_paquete* paq = crear_paquete();
//     t_paquete_entre* paquete = malloc(sizeof(t_paquete_entre));
//     paquete->operacion = operacion;
//     paquete->payload = payload;
//     agregar_a_paquete(paq, paquete, sizeof(t_paquete_entre));
//     enviar_paquete(paq,puerto_cpu_dispatch);
//     log_info(logger, "PAQUETE CREADO Y ENVIADO A CPU DISPATCH");
// }
// void interrumpir(void)
// {
//     t_paquete *paquete_fin_de_q = crear_paquete;
//     t_paquete_entre *fin_q = malloc(sizeof(t_paquete_entre));
//     fin_q->operacion = INTERRUMPIR_PROCESO;
//     agregar_a_paquete(paquete_fin_de_q, fin_q, sizeof(t_paquete_entre));
//     enviar_paquete(paquete_fin_de_q, resultHandshakeInterrupt);
//     eliminar_paquete(paquete_fin_de_q);
// }

