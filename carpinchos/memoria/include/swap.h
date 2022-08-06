#ifndef VIRTUAL_H_
#define VIRTUAL_H_

#include <paquete.h>
#include <constantes.h>
#include <commons/collections/list.h>

int conexion_swap;
extern int tamanio_pagina;

int swap_iniciar_carpincho(int pid);
void* swap_leer_pagina(int pid, int pagina);
int swap_escribir_pagina(int pid, int pagina, void* contenido);
int swap_eliminar_pagina(int pid, int pagina);
void swap_eliminar_proceso(int pid);

#endif