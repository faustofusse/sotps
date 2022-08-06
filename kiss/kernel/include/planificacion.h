#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "colas.h"

void iniciar_planificacion();
void finalizar_planificacion();
void iniciar_proceso(uint32_t tamanio, char* instrucciones, int conexion);

#endif
