#include "main.h"

int main(int argc, char** argv) {
    char* config_path = "config.cfg";
    // Agarro los parametros
    if (argc > 1) config_path = argv[1];
    // Inicio el logger
    logger = log_create("memoria.log", "memoria", true, LOG_LEVEL_INFO);
    // Leo la configuracion
    leer_config(config_path);
    // Inicializo la memoria
    memoria_inicializar();
    // Inicializo la swap
    swap_inicializar();
    // Creo el servidor
    int servidor = crear_servidor(memoria_config.puerto_escucha);
    // Creo los handlers
    t_list* handlers = crear_handlers();
    // Escucho
    recibir_conexiones(servidor, handlers, logger, false);
    // Termino el programa
    return EXIT_SUCCESS;
}
