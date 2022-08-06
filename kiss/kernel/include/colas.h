#ifndef COLAS_H_
#define COLAS_H_

#include "memoria.h"

extern t_list *colas[CANTIDAD_ESTADOS];
extern sem_t semaforos[CANTIDAD_ESTADOS];

void sacar_proceso(int cola, t_proceso* p);
void push(int cola, t_proceso *p);
t_proceso* pop(int cola);
t_proceso* peek(int cola);
void insert_sorted(int cola, t_proceso* p);

#endif
