#ifndef KERNEL_MEMORIA_H_
#define KERNEL_MEMORIA_H_

#include "cpu.h"

extern int conexion_memoria;

void conectar_memoria();
void iniciar_en_memoria(t_proceso* p);
void finalizar_en_memoria(t_proceso* p);
void suspender_en_memoria(t_proceso* p);

#endif
