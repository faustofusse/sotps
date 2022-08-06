#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef struct {
    char* identificador;
    t_list* parametros;
} t_instruccion;

/* char* serializar_instrucciones(t_list* instrucciones); */
t_list* deserializar_instrucciones(char* contenido_archivo);
void destruir_instrucciones(t_list* instrucciones);
/* uint32_t tamanio_instrucciones(t_list* instrucciones); */

#endif
