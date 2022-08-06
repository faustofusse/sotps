#ifndef MEMORIA_PAGINAS
#define MEMORIA_PAGINAS

#include "swap.h"

uint32_t traer_a_memoria(uint32_t tabla_1er_nivel, t_pagina* pagina);
t_pagina* buscar_pagina(uint32_t direccion_fisica);

#endif
