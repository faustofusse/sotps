#ifndef MMU_H_
#define MMU_H_

#include "tlb.h"

extern t_pcb *pcb;

extern uint32_t entradas_por_tabla, tamanio_pagina;

extern int conexion_memoria;

void conectar_memoria();
void memoria_write(uint32_t direccion_logica, uint32_t valor);
uint32_t memoria_read(uint32_t direccion_logica);
uint32_t traducir(uint32_t direccion_logica);

#endif
