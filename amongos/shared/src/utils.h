#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <string.h>

typedef struct {
	int size;
	void* stream;
} t_buffer;

void destruir_buffer(t_buffer* buffer);
void verificar(int resultado, char* mensaje);
void separar_string(t_list* lista, char* cadena, char separador[]);
char* lista_a_string(t_list* lista, char* separador);

#endif
