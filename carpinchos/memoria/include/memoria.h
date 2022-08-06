#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <paquete.h>
#include <constantes.h>

#include "paginacion.h"

t_config* config;

t_config* crear_config();
t_log* crear_logger();
void recibir(int, int);
int conectar_swamp(t_config* config);

#endif