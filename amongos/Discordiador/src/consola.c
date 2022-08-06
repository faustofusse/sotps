#include "consola.h"

void iniciar_consola() {
	t_list* parametros = list_create();
	read_history("./.history");
	char* linea;
	while (1) {
		linea = readline("> ");

		if (!strncmp(linea, "exit", 4) || !strncmp(linea, "EXIT", 4)) {
			free(linea);
			break;
		} else if (!strncmp(linea, "", 1)) {
			free(linea);
			continue;
		} else if (!strncmp(linea, "clear", 5)) {
			free(linea);
			system("clear");
			continue;
		} 

		sem_wait(&mutex_emergencia);
		if (emergencia) {
			free(linea);
			printf("No se pueden ejecutar comandos, se esta resolviendo el sabotaje.\n");
			sem_post(&mutex_emergencia);
			continue;
		}
		sem_post(&mutex_emergencia);

		add_history(linea);

		separar_string(parametros, linea, " ");

		ejecutar_comando(parametros);

		free(linea);
	}
	write_history("./.history");
	list_destroy(parametros);
}

int obtener_comando(char* comando) {
	for (int i = 0; i < sizeof(comandos_string)/sizeof(char*); i++)
		if (strcmp(comando, comandos_string[i]) == 0)
			return i;
	return -1;
}

void ejecutar_comando(t_list* parametros) {
	t_list* primero = list_take_and_remove(parametros, 1);
	char* accion = (char*) primero->head->data;
	int comando = obtener_comando(accion);
	switch(comando) {
		case INICIAR_PATOTA:
			if (list_size(parametros) < 2) { printf("%s\n", "Error: Faltan parametros"); break; }
			int cantidad_tripulantes = atoi(list_get(parametros, 0));
			if (cantidad_tripulantes <= 0) { printf("%s\n", "Error: Ingrese un numero de tripulantes > 0"); break; }
			char* nombre_archivo = (char*) list_get(parametros, 1);
			list_remove(parametros, 0);
			list_add(parametros, "0|0");
			list_remove(parametros, 0);
			int cantidad_de_posiciones = list_size(parametros);
			if(cantidad_de_posiciones > 1) {
				list_remove(parametros, cantidad_de_posiciones - 1);
			}
			t_list* posiciones = list_take_and_remove(parametros, cantidad_tripulantes);
			iniciar_patota(cantidad_tripulantes, nombre_archivo, posiciones);
			break;
		case EXPULSAR_TRIPULANTE:
			if (list_size(parametros) < 2) printf("%s\n", "Error: Faltan parametros");
			else expulsar_tripulante(atoi(list_get(parametros, 0)), atoi(list_get(parametros, 1)));
			break;
		case OBTENER_BITACORA:
			if (list_size(parametros) < 2) printf("%s\n", "Error: Faltan parametros");
			else obtener_bitacora(atoi(list_get(parametros, 0)), atoi(list_get(parametros, 1)));
			break;
		case LISTAR_TRIPULANTES: listar_tripulantes(); break;
		case INICIAR_PLANIFICACION: iniciar_planificacion(); break;
		case PAUSAR_PLANIFICACION: pausar_planificacion(); break;
		case -1:
		default:
			printf("Comando no encontrado :(\n");
			break;
	}
	list_destroy(primero);
}
