#ifndef SWAP_H_
#define SWAP_H_

#include "types.h"

void swap_inicializar();

void swap_iniciar_proceso(t_proceso* proceso);
void swap_suspender_proceso(t_proceso* proceso);
void swap_finalizar_proceso(t_proceso* proceso);
void swap_escribir_pagina(t_proceso* proceso, uint32_t pagina, void* contenido);
void* swap_leer_pagina(t_proceso* proceso, uint32_t pagina);

// TODO meter en otro lado
uint32_t cantidad_paginas(t_proceso* proceso);
uint32_t cantidad_tablas(uint32_t cantidad_paginas);

#endif
