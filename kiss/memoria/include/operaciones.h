#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "paginas.h"

void memoria_inicializar();
uint32_t memoria_leer(uint32_t direccion_fisica);
bool memoria_escribir(uint32_t direccion_fisica, uint32_t valor);
uint32_t memoria_pedir_tabla(uint32_t tabla_1er_nivel, uint32_t pagina);
uint32_t memoria_pedir_marco(uint32_t tabla_1er_nivel, uint32_t tabla_2do_nivel, uint32_t pagina);

#endif
