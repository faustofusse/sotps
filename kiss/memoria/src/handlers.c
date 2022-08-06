#include "handlers.h"

void handler_iniciar_proceso(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t pid = *((uint32_t*) list_get(valores, 0));
    uint32_t tamanio = *((uint32_t*) list_get(valores, 1));
    uint32_t tabla_paginas = iniciar_proceso(pid, tamanio);
    log_info(logger, "Iniciando el proceso %d", pid);
    list_destroy_and_destroy_elements(valores, free);
    enviar_numero(conexion, tabla_paginas);
}

void handler_finalizar_proceso(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t tabla = *((uint32_t*) list_get(valores, 0));
    finalizar_proceso(tabla);
    list_destroy_and_destroy_elements(valores, free);
    enviar_numero(conexion, FINALIZAR_PROCESO);
}

void handler_suspender_proceso(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t tabla = *((uint32_t*) list_get(valores, 0));
    suspender_proceso(tabla);
    list_destroy_and_destroy_elements(valores, free);
    enviar_numero(conexion, SUSPENDER_PROCESO);
}

void handler_leer(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t direccion_fisica = *((uint32_t*) list_get(valores, 0));
    list_destroy_and_destroy_elements(valores, free);
    usleep(memoria_config.retardo_memoria * 1000);
    uint32_t valor = memoria_leer(direccion_fisica);
    log_info(logger, "READ (dir fisica = %d) -> %d", direccion_fisica, valor);
    enviar_numero(conexion, valor);
}

void handler_escribir(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t direccion_fisica = *((uint32_t*) list_get(valores, 0));
    uint32_t valor = *((uint32_t*) list_get(valores, 1));
    list_destroy_and_destroy_elements(valores, free);
    usleep(memoria_config.retardo_memoria * 1000);
    log_info(logger, "WRITE (dir fisica = %d) -> %d", direccion_fisica, valor);
    enviar_numero(conexion, memoria_escribir(direccion_fisica, valor) ? ESCRITURA_OK : ESCRITURA_ERROR);
}

void handler_pedir_tabla(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t tabla_1er_nivel = *((uint32_t*) list_get(valores, 0));
    uint32_t pagina = *((uint32_t*) list_get(valores, 1));
    list_destroy_and_destroy_elements(valores, free);
    usleep(memoria_config.retardo_memoria * 1000);
    uint32_t tabla_2do_nivel = memoria_pedir_tabla(tabla_1er_nivel, pagina);
    log_info(logger, "Tabla de 2do nivel de (%d | %d) = %d", tabla_1er_nivel, pagina, tabla_2do_nivel);
    enviar_numero(conexion, tabla_2do_nivel);
}

void handler_pedir_marco(int conexion, t_log* logger) {
    log_info(logger, "-------------------------------");
    t_list* valores = recibir_paquete(conexion);
    uint32_t tabla_1er_nivel = *((uint32_t*) list_get(valores, 0));
    uint32_t tabla_2do_nivel = *((uint32_t*) list_get(valores, 1));
    uint32_t pagina = *((uint32_t*) list_get(valores, 2));
    list_destroy_and_destroy_elements(valores, free);
    usleep(memoria_config.retardo_memoria * 1000);
    uint32_t marco = memoria_pedir_marco(tabla_1er_nivel, tabla_2do_nivel, pagina);
    log_info(logger, "Marco (%d | %d | %d) = %d", tabla_1er_nivel, tabla_2do_nivel, pagina, marco);
    enviar_numero(conexion, marco);
}

void recibir_handshake_kernel(int conexion, t_log* logger) {
    enviar_numero(conexion, HANDSHAKE_MEMORIA);
}

void recibir_handshake_cpu(int conexion, t_log* logger) {
    t_paquete* paquete = crear_paquete(HANDSHAKE_MEMORIA);
    agregar_a_paquete(paquete, &memoria_config.tamanio_pagina, sizeof(uint32_t));
    agregar_a_paquete(paquete, &memoria_config.entradas_por_tabla, sizeof(uint32_t));
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

t_list* crear_handlers() {
    t_list* handlers = list_create();
    agregar_handler(handlers, HANDSHAKE_KERNEL, recibir_handshake_kernel);
    agregar_handler(handlers, HANDSHAKE_CPU, recibir_handshake_cpu);
    agregar_handler(handlers, PEDIR_TABLA, handler_pedir_tabla);
    agregar_handler(handlers, PEDIR_MARCO, handler_pedir_marco);
    agregar_handler(handlers, MEMORIA_ESCRIBIR, handler_escribir);
    agregar_handler(handlers, MEMORIA_LEER, handler_leer);
    agregar_handler(handlers, INICIAR_PROCESO, handler_iniciar_proceso);
    agregar_handler(handlers, SUSPENDER_PROCESO, handler_suspender_proceso);
    agregar_handler(handlers, FINALIZAR_PROCESO, handler_finalizar_proceso);
    return handlers;
}

