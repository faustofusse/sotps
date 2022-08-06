#ifndef ACCIONES_H_
#define ACCIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <paquete.h>
#include <utils.h>
#include <semaphore.h>
#include <constantes.h>
#include <commons/log.h>
#include <commons/config.h>
#include <signal.h>

#include "constantes.h"
#include "paginacion.h"
#include "segmentacion.h"

extern t_log* logger;
int esquema;
void* memoria_principal;

bool iniciar_patota(u_int32_t pid, char* tareas_string, int cantidad_tripulantes, char* posiciones_string, int caracter);
void enviar_proxima_tarea(u_int32_t tid, u_int32_t pid, int cliente);
void cambiar_estado(u_int32_t tid, u_int32_t pid, char estado);
void expulsar(u_int32_t tid, u_int32_t pid);
void mover(u_int32_t tid, u_int32_t pid, u_int32_t x, u_int32_t y);

void recibir(int cliente, int operacion);
void iniciar_memoria(t_config* config);

#endif