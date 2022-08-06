#include "config.h"

t_mem_config memoria_config;

void leer_config(char* path) {
    t_config* config = config_create(path);
    memoria_config.mostrar_algoritmo = getenv("MOSTRAR_ALGORITMO") != NULL;
    const char* puerto_escucha = getenv("PUERTO_ESCUCHA");
    memoria_config.puerto_escucha = puerto_escucha != NULL ? atoi(puerto_escucha)
        : config_get_int_value(config, "PUERTO_ESCUCHA");
    memoria_config.tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
    memoria_config.tamanio_pagina = config_get_int_value(config, "TAM_PAGINA");
    memoria_config.entradas_por_tabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
    memoria_config.retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
    char* algoritmo_reemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    memoria_config.algoritmo_reemplazo = strcmp(algoritmo_reemplazo, "CLOCK") == 0 ? CLOCK : CLOCK_M;
    memoria_config.marcos_por_proceso = config_get_int_value(config, "MARCOS_POR_PROCESO");
    memoria_config.retardo_swap = config_get_int_value(config, "RETARDO_SWAP");
    char* path_swap = config_get_string_value(config, "PATH_SWAP");
    memoria_config.path_swap = malloc(strlen(path_swap) + 1);
    memcpy(memoria_config.path_swap, path_swap, strlen(path_swap));
    memoria_config.path_swap[strlen(path_swap)] = '\0';
    config_destroy(config);
    printf("MEMORIA=%d\n", memoria_config.puerto_escucha);
}
