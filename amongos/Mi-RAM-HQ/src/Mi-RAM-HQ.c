#include "Mi-RAM-HQ.h"

int main(void) {
	logger = crear_logger();
	config = crear_config();

	iniciar_memoria(config);

	if (MAPA_ACTIVADO) mapa_iniciar();

	int servidor = crear_servidor(config_get_int_value(config, "PUERTO"));

	log_info(logger, "El servidor esta escuchando...");

	escuchar(servidor, logger, *recibir);

	return EXIT_SUCCESS;
}

t_config* crear_config() {
	return config_create("./Mi-RAM-HQ.config");
}

t_log* crear_logger() {
	return log_create("./Mi-RAM-HQ.log", "Mi-RAM-HQ", !MAPA_ACTIVADO, LOG_LEVEL_INFO);
}

void terminar_programa() {
	log_destroy(logger);
	config_destroy(config);
}