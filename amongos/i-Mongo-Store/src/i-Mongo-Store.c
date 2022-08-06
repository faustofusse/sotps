#include "i-Mongo-Store.h"

void signal_handler(int sig) {
	if (sig != SIGUSR1) return;
	if (socket_discordiador == -1) {
		log_info(logger, "Recivi la señal pero todavia no se conecto el discordiador.");
		return;
	}
	int x = 3, y = 7;
	t_paquete* paquete = crear_paquete(SABOTAJE);
	agregar_a_paquete(paquete, &x, sizeof(int));
	agregar_a_paquete(paquete, &y, sizeof(int));
	enviar_paquete(paquete, socket_discordiador);
	eliminar_paquete(paquete);
}

int main(void) {
	logger = crear_logger();
	config = crear_config();
	socket_discordiador = -1;

	// obtener_variables_de_config();

	signal(SIGUSR1, signal_handler);

	int servidor = crear_servidor(config_get_int_value(config, "PUERTO"));

	log_info(logger, "El servidor esta escuchando...");

	escuchar(servidor, logger, *recibir);

	return EXIT_SUCCESS;
}

void recibir(int cliente, int operacion){
	switch (operacion) {
		case HANDSHAKE_DISCORDIADOR:
			log_info(logger, "HOLA DISCORDIADOR!");
			socket_discordiador = cliente;
			break;
		// HANDSHAKE_TRIPULANTE
		case INICIAR_FSCK:
			log_info(logger, "INICIO FSCK!");
			break;
		case OBTENER_BITACORA:
			log_info(logger, "TOMA LA BITACORA PIBE!");
			break;
		case AGREGAR_ACCION:
			log_info(logger, "AGREGAR ACCION A LA BITACORA!");
			break;
		default:
			log_info(logger, "Operacion no reconocida!");
			break;
	}
}

t_config* crear_config() {
	return config_create("./i-Mongo-Store.config");
}

t_log* crear_logger() {
	return log_create("./i-Mongo-Store.log", "i-Mongo-Store", true, LOG_LEVEL_INFO);
}

void terminar_programa() {
	log_destroy(logger);
	config_destroy(config);
}

char* sacar_corchetes(char* cadena) {
	int size = strlen(cadena) + 1 - 2;
	char* resultado = malloc(size);
	memcpy(resultado, cadena + 1, size - 1);
	return resultado;
}

void obtener_variables_de_config() {
	punto_montaje = config_get_string_value(config, "PUNTO_MONTAJE");
	tiempo_sincronizacion = config_get_int_value(config, "TIEMPO_SINCRONIZACION");
	// Separo el string (SIMPLIFICAR ESTO)
	char* posiciones_string = config_get_string_value(config, "POSICIONES_SABOTAJE");
	t_list* temp = list_create();
	posiciones_sabotaje = list_create();
	char* sub = string_substring(posiciones_string, 1, strlen(posiciones_string) - 2);
	separar_string(temp, sub, ",");
	for (int i = 0; i < list_size(temp); i++) {
		char* elemento = (char*) list_get(temp, i);
		list_add(posiciones_sabotaje, elemento[0] == ' ' ? string_substring(elemento, 1, 3) : elemento);
	}
	for (int i = 0; i<list_size(posiciones_sabotaje); i++) printf("%s\n", (char*) list_get(posiciones_sabotaje, i));
	list_destroy(temp);
}