#ifndef TLB_H
#define TLB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <constantes.h>

#include "types.h"

uint32_t hits, misses;
int entradas_tlb, retardo_hit, retardo_miss, algoritmo_tlb;
char* path_dump;

t_list* tlb;

void iniciar_tlb();
void limpiar_tlb();
void destruir_tlb();
void sacar_de_tlb(t_pagina* pagina);
void sacar_carpincho_de_tlb(int pid);
void agregar_a_tlb(t_pagina* pagina);
uint32_t buscar_en_tlb(t_carpincho* carpincho, uint32_t numero);
void dump_tlb();

void pruebas_tlb();

#endif