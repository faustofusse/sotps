#include "proceso.h"

t_log* logger;
t_list* primer_nivel, *segundo_nivel;
void* memoria_principal;

t_proceso* crear_proceso(uint32_t pid, uint32_t tamanio) {
    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->pid = pid;
    proceso->tamanio = tamanio;
    proceso->tablas = malloc(memoria_config.entradas_por_tabla * sizeof(uint32_t));
    for (int i = 0; i < memoria_config.entradas_por_tabla; i++)
        proceso->tablas[i] = -1;
    proceso->swap = NULL;
    proceso->residentes = list_create();
    return proceso;
}

t_pagina* crear_pagina(uint32_t numero) {
    t_pagina* pagina = malloc(sizeof(t_pagina));
    pagina->numero = numero;
    pagina->marco = 0;
    pagina->modificado = 0;
    pagina->presencia = 0;
    pagina->uso = 0;
    return pagina;
}

void crear_paginas(t_pagina** tabla, uint32_t numero_tabla, uint32_t cantidad) {
    for (int i = 0; i < cantidad; i++) tabla[i] = crear_pagina(numero_tabla * memoria_config.entradas_por_tabla + i);
}

uint32_t min(uint32_t a, uint32_t b) {
    return a > b ? b : a;
}

uint32_t entradas_tabla_intermedia(uint32_t numero_tabla, uint32_t cant_paginas, uint32_t cant_entradas) {
    if (numero_tabla == 0) return min(cant_entradas, cant_paginas);
    if (cant_paginas % cant_entradas == 0) return cant_entradas;
    return cant_paginas % cant_entradas;
}

void crear_tablas(t_proceso* proceso) {
    uint32_t cant_entradas = memoria_config.entradas_por_tabla;
    uint32_t cant_paginas = cantidad_paginas(proceso);
    uint32_t cant_tablas = cantidad_tablas(cant_paginas);
    for (int i = 0; i < cant_tablas; i++) {
        t_pagina** tabla = malloc(sizeof(t_pagina*) * cant_entradas);
        uint32_t numero_tabla = list_size(segundo_nivel);
        proceso->tablas[i] = numero_tabla;
        crear_paginas(tabla, i, entradas_tabla_intermedia(i, cant_paginas, cant_entradas));
        list_add(segundo_nivel, tabla);
    }
}

uint32_t iniciar_proceso(uint32_t pid, uint32_t tamanio) {
    uint32_t tamanio_maximo = memoria_config.entradas_por_tabla * memoria_config.entradas_por_tabla * memoria_config.tamanio_pagina;
    if (tamanio > tamanio_maximo) {
        log_error(logger, "Proceso con tamanio invalido %d (tamanio maximo: %d)", tamanio, tamanio_maximo);
        return -1;
    }
    pthread_mutex_lock(&mutex_tablas);
    uint32_t numero_tabla = list_size(primer_nivel);
    t_proceso* proceso = crear_proceso(pid, tamanio);
    list_add(primer_nivel, proceso);
    crear_tablas(proceso);
    pthread_mutex_unlock(&mutex_tablas);
    swap_iniciar_proceso(proceso);
    return numero_tabla;
}

void suspender_proceso(uint32_t tabla_1er_nivel) {
    pthread_mutex_lock(&mutex_tablas);
    if (tabla_1er_nivel >= list_size(primer_nivel)) {
        log_error(logger, "No se encontro el proceso con la tabla de 1er nivel = %d", tabla_1er_nivel);
        return;
    }
    t_proceso* proceso = (t_proceso*) list_get(primer_nivel, tabla_1er_nivel);
    log_info(logger, "Se suspende el proceso %d", proceso->pid);
    if (proceso == NULL) {
        log_error(logger, "No se encontro el proceso con la tabla de 1er nivel = %d", tabla_1er_nivel);
        return;
    }
    swap_suspender_proceso(proceso);
    pthread_mutex_unlock(&mutex_tablas);
}

void finalizar_proceso(uint32_t tabla_1er_nivel) {
    pthread_mutex_lock(&mutex_tablas);
    if (tabla_1er_nivel >= list_size(primer_nivel)) {
        log_error(logger, "No se encontro el proceso con la tabla de 1er nivel = %d", tabla_1er_nivel);
        return;
    }
    t_proceso* proceso = (t_proceso*) list_get(primer_nivel, tabla_1er_nivel);
    log_info(logger, "Finaliza el proceso %d", proceso->pid);
    if (proceso == NULL) {
        log_error(logger, "No se encontro el proceso con la tabla de 1er nivel = %d", tabla_1er_nivel);
        return;
    }
    // pongo los bits de marcos en 0
    while (!list_is_empty(proceso->residentes)) 
        bitarray_clean_bit(marcos_libres, ((t_pagina*) list_remove(proceso->residentes, 0))->marco);
    // lo finalizo en swap
    swap_finalizar_proceso(proceso);
    free(proceso->tablas);
    list_clean_and_destroy_elements(proceso->residentes, free);
    pthread_mutex_unlock(&mutex_tablas);
}

