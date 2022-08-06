#ifndef PAGINACION_H
#define PAGINACION_H

#include <string.h>

#include "pagina.h"

void iniciar_memoria();
void iniciar_carpincho(uint32_t pid);
void eliminar_carpincho(uint32_t pid);

int memfree(uint32_t pid, mate_pointer addr);
void* memread(uint32_t pid, mate_pointer addr, int size);
int memwrite(uint32_t pid, void *contenido, mate_pointer addr, int size);
mate_pointer memalloc(uint32_t pid, int size);

void dump_memoria();

#endif