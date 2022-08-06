#ifndef I_MONGO_STORE_H_
#define I_MONGO_STORE_H_

#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <signal.h>
#include <constantes.h>
#include <paquete.h>

int socket_discordiador;

t_log* logger;
t_config* config;

char* punto_montaje;
int tiempo_sincronizacion;
t_list* posiciones_sabotaje;

t_config* crear_config();
t_log* crear_logger();
void recibir(int cliente, int operacion);
void terminar_programa();
void obtener_variables_de_config();

#endif