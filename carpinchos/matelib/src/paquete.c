#include "paquete.h"

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

void* recibir_buffer(int* size, int cliente)
{
	void * buffer;
	verificar( recv(cliente, size, sizeof(int), MSG_WAITALL) , "Error en el recv" );
	buffer = malloc(*size);
	verificar( recv(cliente, buffer, *size, MSG_WAITALL) , "Error en el recv" );
	return buffer;
}

void reenviar_paquete(int desde, int hacia, int operacion)
{
	int size;
	send(hacia, &operacion, sizeof(int), MSG_NOSIGNAL);
	t_buffer* buffer = recibir_buffer(&size, desde);
	send(hacia, &size, sizeof(int), MSG_NOSIGNAL);
	send(hacia, buffer, size, MSG_NOSIGNAL);
	free(buffer);
}

t_list* recibir_paquete(int cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;
	buffer = recibir_buffer(&size, cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
	return NULL;
}

t_paquete* crear_paquete(int operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	verificar( send(cliente, a_enviar, bytes, MSG_NOSIGNAL), "Error al hacer el send del paquete" );

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
