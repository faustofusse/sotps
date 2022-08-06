#ifndef PAQUETE_H_
#define PAQUETE_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <commons/collections/list.h>
#include "utils.h"
#include "constantes.h"

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_paquete* crear_paquete(int operacion);
t_list* recibir_paquete(int cliente);
void* recibir_buffer(int* size, int cliente);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void agregar_archivo_a_paquete(t_paquete* paquete, FILE* archivo, char* separador);
void enviar_paquete(t_paquete* paquete, int cliente);
void eliminar_paquete(t_paquete* paquete);
void enviar_numero(int cliente, int numero);

#endif