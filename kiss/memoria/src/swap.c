#include "swap.h"

void swap_inicializar() {
    struct stat st = {0};
    if (stat(memoria_config.path_swap, &st) == -1)
        mkdir(memoria_config.path_swap, 0700);
}

uint32_t ceiling(uint32_t dividendo, uint32_t divisor) {
    return (dividendo + divisor - 1) / divisor;
}

uint32_t cantidad_paginas(t_proceso* proceso) {
    return ceiling(proceso->tamanio, memoria_config.tamanio_pagina);
} 

uint32_t cantidad_tablas(uint32_t cantidad_paginas) {
    return ceiling(cantidad_paginas, memoria_config.entradas_por_tabla);
}

char* get_path(t_proceso* proceso) {
    return string_from_format("%s/%d.swap", memoria_config.path_swap, proceso->pid);
}

void swap_iniciar_proceso(t_proceso* proceso) {
    pthread_mutex_lock(&mutex_tablas);
    char* path = get_path(proceso);
    uint32_t tamanio = proceso->tamanio;
    pthread_mutex_unlock(&mutex_tablas);
    // Abro el archivo
    int archivo = open(path, O_RDWR | O_CREAT, 0666);
    if (archivo == -1) {
        log_error(logger, "No se pudo abrir el archivo %s", path);
        free(path);
        return;
    }
    // Le cambio el tamaño y lo mapeo
    ftruncate(archivo, tamanio);
    void* contenido = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);
    // Me fijo si fallo
    if (contenido == MAP_FAILED) {
        log_error(logger, "No se pudo mapear el archivo %s", path);
    } else {
        memset(contenido, 0, tamanio);
        pthread_mutex_lock(&mutex_tablas);
        proceso->swap = contenido;
        pthread_mutex_unlock(&mutex_tablas);
    }
    // Libero memoria
    close(archivo);
    free(path);
}

void swap_suspender_proceso(t_proceso* proceso) {
    while(!list_is_empty(proceso->residentes)) {
        t_pagina* pagina = list_remove(proceso->residentes, 0);
        if (pagina->modificado == 1) {
            swap_escribir_pagina(proceso, pagina->numero, memoria_principal + pagina->marco * memoria_config.tamanio_pagina);
        }
        pagina->presencia = 0;
        bitarray_clean_bit(marcos_libres, pagina->marco);
    }
}

void swap_finalizar_proceso(t_proceso* proceso) {
    if (munmap(proceso->swap, proceso->tamanio) != 0) {
        log_error(logger, "No se pudo unmapear el proceso %d", proceso->pid);
        return;
    }
    char* path = get_path(proceso);
    int removed = remove(path);
    free(path);
    if (removed != 0) {
        log_error(logger, "No se pudo eliminar el archivo del proceso %d", proceso->pid);
        return;
    }
}

void* swap_leer_pagina(t_proceso* proceso, uint32_t pagina) {
    pthread_mutex_lock(&mutex_swap);
    void* contenido = malloc(memoria_config.tamanio_pagina);
    memcpy(contenido, proceso->swap + pagina * memoria_config.tamanio_pagina, memoria_config.tamanio_pagina);
    log_info(logger, "SWAP -> leyendo pagina %d del P%d", pagina, proceso->pid);
    /* usleep(memoria_config.retardo_swap * 1000); */
    pthread_mutex_unlock(&mutex_swap);
    return contenido;
}

void swap_escribir_pagina(t_proceso* proceso, uint32_t pagina, void* contenido) {
    pthread_mutex_lock(&mutex_swap);
    memcpy(proceso->swap + pagina * memoria_config.tamanio_pagina, contenido, memoria_config.tamanio_pagina);
    msync(proceso->swap, proceso->tamanio, MS_SYNC);
    log_info(logger, "SWAP -> escribiendo pagina %d del P%d", pagina, proceso->pid);
    /* usleep(memoria_config.retardo_swap * 1000); */
    pthread_mutex_unlock(&mutex_swap);
}
