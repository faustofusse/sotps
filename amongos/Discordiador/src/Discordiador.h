#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <commons/log.h>
#include <commons/config.h>
#include <constantes.h>

#include "consola.h" // discordiador <- consola <- tripulante <- sabotaje <- planificacion
// #include "planificacion.h"

t_log* logger;
t_config* config;

int socket_ram, socket_store;

t_config* crear_config();
t_log* crear_logger();
void terminar_programa();
void inicializar_variables_planificacion();
int conectarse_a_store();
int conectarse_a_ram();

#endif
