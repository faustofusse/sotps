#ifndef PAGINA_H_
#define PAGINA_H_

#include "types.h"
#include "swap.h"
#include "tlb.h"

t_pagina* reemplazar_pagina_lru(t_list* lista, t_pagina* nueva);
t_pagina* reemplazar_pagina_clock(t_list* lista, t_pagina* nueva);
int reemplazar_pagina(t_list* lista, t_pagina* nueva);
int traer_pagina(t_carpincho* carpincho, int numero);
int buscar_pagina(t_carpincho* carpincho, int numero);
void eliminar_pagina(t_carpincho* carpincho, int numero);
void insertar_pagina(t_carpincho* carpincho, t_pagina* pagina, void* contenido);

#endif