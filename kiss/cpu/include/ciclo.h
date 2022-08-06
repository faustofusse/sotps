#ifndef CICLO_H_
#define CICLO_H_

#include "execute.h"

void fetch();
void decode(t_instruccion* instruccion);
t_list* fetch_operands(t_instruccion* instruccion);
void execute(t_instruccion* instruccion, t_list* operands, t_executable ejecutable);
void check_interrupt();

#endif
