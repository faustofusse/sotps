#ifndef MEM_CONFIG_H_
#define MEM_CONFIG_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <constantes.h>
#include <paquete.h>
#include <socket.h>

typedef struct {
    uint32_t puerto_escucha;
    uint32_t tamanio_memoria;
    uint32_t tamanio_pagina;
    uint32_t entradas_por_tabla;
    uint32_t retardo_memoria;
    uint32_t algoritmo_reemplazo;
    uint32_t marcos_por_proceso;
    uint32_t retardo_swap;
    bool mostrar_algoritmo;
    char* path_swap;
} t_mem_config;

extern t_mem_config memoria_config;

void leer_config(char* path);

#endif
