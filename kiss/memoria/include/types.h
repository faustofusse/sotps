#ifndef MEMORIA_TYPES_H_
#define MEMORIA_TYPES_H_

#include "config.h"

typedef struct {
    uint32_t numero;
    uint32_t marco;
    uint8_t presencia;
    uint8_t uso;
    uint8_t modificado;
} t_pagina;

typedef struct {
    uint32_t pid;
    uint32_t tamanio;
    uint32_t* tablas;
    void* swap;
    t_list* residentes; // lista de t_pagina
} t_proceso;

extern t_list *primer_nivel, // lista de t_proceso
              *segundo_nivel; // lista de arrays de t_pagina

extern t_bitarray* marcos_libres;

extern void *memoria_principal;

extern t_log *logger;

extern pthread_mutex_t mutex_tablas, mutex_memoria, mutex_swap;

#endif
