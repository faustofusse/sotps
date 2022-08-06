#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_

#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <commons/log.h>
#include <commons/config.h>

#include "mapa.h"
#include "acciones.h"

t_log* logger;
t_config* config;

char* path_swap;
int esquema, tamanio_pag, tamanio_swap, algoritmo_reemplazo, criterio_seleccion;

t_config* crear_config();
t_log* crear_logger();
void terminar_programa();
// void iniciar_listas();
// void iniciar_variables();

#endif