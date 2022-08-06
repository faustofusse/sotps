#include "utils.h"

void verificar(int resultado, char* mensaje) 
{
	if (resultado == -1) {
		printf("%s\n", mensaje);
		exit(EXIT_FAILURE);
	}
}

char* lista_a_string(t_list* lista, char* separador) 
{
    char* resultado = NULL;
    char* nuevo = NULL;
    int tamanio = 0;
    for (int i = 0; i < list_size(lista); i++) {
        nuevo = (char*) list_get(lista, i);
        tamanio += strlen(nuevo) + 1; // el +1 es para el separador o el /0 si es el ultimo
        resultado = resultado == NULL ? malloc(tamanio) : realloc(resultado, tamanio); 
        memcpy(resultado + tamanio - strlen(nuevo) - 1, nuevo, strlen(nuevo));
        resultado[tamanio - 1] = (i == list_size(lista)-1) ? '\0' : separador[0];
    }
    return resultado;
}

void separar_string(t_list* lista, char* cadena, char* separador) 
{
	list_clean(lista);
	char *token = strtok(cadena, separador);
	while( token != NULL ) {
		list_add(lista, token);
		token = strtok(NULL, separador);
	}
}

void destruir_buffer(t_buffer* buffer) 
{
    free(buffer->stream);
    free(buffer);
}
