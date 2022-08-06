#ifndef PROCESO_H_
#define PROCESO_H_

#include "operaciones.h"

uint32_t iniciar_proceso(uint32_t pid, uint32_t tamanio);
void suspender_proceso(uint32_t pid);
void finalizar_proceso(uint32_t pid);

#endif
