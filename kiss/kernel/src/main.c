#include "main.h"
#include "config.h"
#include "cpu.h"
#include "socket.h"

void signal_handler(int signum) {
    if (signum != SIGINT) return; 
    finalizar_planificacion();
    enviar_numero(conexion_memoria, CLOSE_CONNECTION);
    enviar_numero(conexion_dispatch, CLOSE_CONNECTION);
    enviar_numero(conexion_interrupt, CLOSE_CONNECTION);
    exit(EXIT_SUCCESS);
}

void handshake_consola(int conexion, t_log* logger) {
    enviar_numero(conexion, HANDSHAKE_KERNEL);
}

void recibir_proceso(int conexion, t_log* logger) {
    t_list* valores = recibir_paquete(conexion);
    char* instrucciones = (char*) list_remove(valores, 0);
    uint32_t tamanio = *((uint32_t*) list_get(valores, 0));
    iniciar_proceso(tamanio, instrucciones, conexion);
    list_destroy_and_destroy_elements(valores, free);
    pthread_exit(NULL);
}

t_list* crear_handlers() {
    t_list* handlers = list_create();
    agregar_handler(handlers, HANDSHAKE_CONSOLA, &handshake_consola);
    agregar_handler(handlers, INSTRUCCIONES, &recibir_proceso);
    return handlers;
}

int main(int argc, char** argv) {
    char* config_path = "config.cfg";
    // Agarro los parametros
    if (argc > 1) config_path = argv[1];
    // Inicializo el logger
    logger = log_create("kernel.log", "kernel", true, LOG_LEVEL_INFO);
    // Leo la config
    leer_config(config_path);
    // Le digo que hacer cuando se cierre el programa
    signal(SIGINT, signal_handler);
    // Me conecto con la CPU
    conectar_cpu();
    // Me conecto con la memoria
    conectar_memoria();
    // Inicializo la planificacion
    iniciar_planificacion();
    // Creo el servidor
    int servidor = crear_servidor(kernel_config->puerto_escucha);
    // Creo los handlers y escucho
    recibir_conexiones(servidor, crear_handlers(), logger, false);
    // Libero memoria y me voy
    log_destroy(logger);
    return 0;
}
