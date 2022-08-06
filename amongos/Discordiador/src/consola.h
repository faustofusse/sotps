#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/collections/list.h>
#include <utils.h>

#include "tripulante.h"
// #include "planificacion.h"

static const char * const comandos_string[] = {
	[INICIAR_PLANIFICACION] = "INICIAR_PLANIFICACION",
	[PAUSAR_PLANIFICACION] = "PAUSAR_PLANIFICACION",
	[OBTENER_BITACORA] = "OBTENER_BITACORA",
	[INICIAR_PATOTA] = "INICIAR_PATOTA",
	[LISTAR_TRIPULANTES] = "LISTAR_TRIPULANTES",
	[EXPULSAR_TRIPULANTE] = "EXPULSAR_TRIPULANTE"
};

void iniciar_consola(void);
void ejecutar_comando(t_list* parametros);
int obtener_comando(char* comando);

#endif
