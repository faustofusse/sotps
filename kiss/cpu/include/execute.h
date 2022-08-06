#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "mmu.h"

typedef bool (*t_executable)(t_list* parametros, t_list* operands);

t_executable get_executable(char* identificador);
void devolver_proceso(uint32_t estado);

#endif
