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

void agregar_archivo_a_paquete(t_paquete* paquete, FILE* archivo, char* separador)
{
	char* caracteres = NULL;
	char* nuevo = NULL;
	char caracter = fgetc(archivo);
	int tamanio = 0;

	while (!feof(archivo)) {
		tamanio++;
		caracteres = realloc(caracteres, tamanio);
		nuevo = caracter == '\n' ? separador : &caracter;
		memcpy(caracteres + tamanio - 1, nuevo, sizeof(char));
		caracter = fgetc(archivo);
	}

	// Si el ultimo caracter no es igual al separador, agrego un byte mas
	if (strncmp(nuevo, separador, 1) != 0) {
		tamanio++;
		caracteres = realloc(caracteres, tamanio);
	}
	// Le agrego el \0 al final para que sea leible
	caracteres[tamanio - 1] = '\0';

	agregar_a_paquete(paquete, (void*) caracteres, tamanio);

	free(caracteres);
}

void enviar_paquete(t_paquete* paquete, int cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	verificar( send(cliente, a_enviar, bytes, MSG_NOSIGNAL), "Error al hacer el send del paquete" );

	free(a_enviar);
}

void enviar_numero(int cliente, int numero)
{
	verificar( send(cliente, &numero, sizeof(int), MSG_NOSIGNAL), "Error al hacer el send" );
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
