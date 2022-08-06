#include "main.h"

int main(int argc, char** argv) {
    char* config_path = "config.cfg";
    // Agarro los parametros
    if (argc > 1) config_path = argv[1];
    // Inicializo el logger
    logger = log_create("cpu.log", "cpu", true, LOG_LEVEL_INFO);
    // Leo la config
    leer_config(config_path);
    // Inicializo los semaforos y las variables globales
    pthread_mutex_init(&mutex_desalojar, NULL);
    // Inicializo la tlb
    inicializar_tlb();
    // Me conecto con la memoria
    conectar_memoria();
    // Inicio los servidores
    int servidor_interrupt = crear_servidor(cpu_config->puerto_cpu_interrupt);
    int servidor_dispatch = crear_servidor(cpu_config->puerto_cpu_dispatch);
    // Recibo conexiones
    recibir_conexiones(servidor_interrupt, handlers_interrupt(), logger, true);
    recibir_conexiones(servidor_dispatch, handlers_dispatch(), logger, false);
    // Libero memoria y me voy
    log_destroy(logger);
    return EXIT_SUCCESS;
}
