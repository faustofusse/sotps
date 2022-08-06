#ifndef PAQUETE_H_
#define PAQUETE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <commons/collections/list.h>

#include "constantes.h"
#include "socket.h"

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(uint32_t operacion);
t_list* recibir_paquete(int cliente);
void* recibir_buffer(int* size, int cliente);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int cliente);
void eliminar_paquete(t_paquete* paquete);
void reenviar_paquete(int desde, int hacia, uint32_t operacion);

#endif