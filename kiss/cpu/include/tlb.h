#ifndef TLB_H_
#define TLB_H_

#include "./config.h"

void inicializar_tlb();
void vaciar_tlb();
void agregar_a_tlb(uint32_t pagina, uint32_t marco);
uint32_t buscar_en_tlb(uint32_t pagina);

#endif
