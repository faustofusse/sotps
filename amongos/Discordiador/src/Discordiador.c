#include "Discordiador.h"

pthread_t hilo_store;

void accion(int cliente, int operacion) {
	if (operacion != SABOTAJE) return;
	t_list* valores = recibir_paquete(cliente);
	int x = *((int*) list_get(valores, 0));
	int y = *((int*) list_get(valores, 1));
	sabotaje(x, y);
	list_destroy_and_destroy_elements(valores, free);
}

void esperar_emergencia() {
	pthread_create(&hilo_store, NULL, *esperar_mensajes, (void*) argumentos((void*) &accion, socket_store, logger));
	pthread_detach(hilo_store);
}

int main(void) {
	logger = crear_logger();
	config = crear_config();

	socket_store = conectarse_a_store();
	socket_ram = conectarse_a_ram();

	enviar_numero(socket_store, HANDSHAKE_DISCORDIADOR);

	inicializar_variables_planificacion(config);

	esperar_emergencia();

	iniciar_consola();

	terminar_programa();

	return EXIT_SUCCESS;
}

t_config* crear_config() {
	return config_create("./Discordiador.config");
}

t_log* crear_logger() {
	return log_create("./Discordiador.log", "Discordiador", false, LOG_LEVEL_INFO);
}

void terminar_programa() {
	log_destroy(logger);
	config_destroy(config);
	close(socket_ram);
	close(socket_store);
}

int conectarse_a_ram()
{
	return crear_conexion(
		config_get_string_value(config, "IP_MI_RAM_HQ"),
		config_get_int_value(config, "PUERTO_MI_RAM_HQ")
	);
}

int conectarse_a_store()
{
	return crear_conexion(
		config_get_string_value(config, "IP_I_MONGO_STORE"),
		config_get_int_value(config, "PUERTO_I_MONGO_STORE")
	);
}