#include "paginas.h"

t_pagina* buscar_pagina(uint32_t direccion_fisica) {
    uint32_t marco = (uint32_t) (direccion_fisica / memoria_config.tamanio_pagina);
    for (int i = list_size(primer_nivel) - 1; i >= 0; i--) {
        t_proceso* proceso = (t_proceso*) list_get(primer_nivel, i);
        for (int j = 0; j < list_size(proceso->residentes); j++) {
            t_pagina* pagina = (t_pagina*) list_get(proceso->residentes, j);
            if (pagina->marco == marco && pagina->presencia == 1) // aca es al pedo el de presencia
                return pagina;
        }
    }
    return NULL;
}

uint32_t proximo_marco_libre(t_proceso* proceso) {
    uint32_t cantidad_marcos = memoria_config.tamanio_memoria / memoria_config.tamanio_pagina;
    for (int i = 0; i < cantidad_marcos; i++)
        if (!bitarray_test_bit(marcos_libres, i))
            return i;
    return -1;
}

t_pagina* reemplazar_clock(t_proceso* proceso, t_pagina* nueva) {
    t_pagina* reemplazada = list_remove(proceso->residentes, 0);
    if (memoria_config.mostrar_algoritmo) log_info(logger, "Reemplazando por clock");
    // busco (u=0, m=_) y voy poniendo u en 0
    while (reemplazada->uso == 1) {
        if (memoria_config.mostrar_algoritmo)
            log_info(logger, "Actual: pag %d (u=%d,m=%d)", reemplazada->numero, reemplazada->uso, reemplazada->modificado);
        list_add(proceso->residentes, reemplazada);
        reemplazada->uso = 0;
        reemplazada = list_remove(proceso->residentes, 0);
    }
    return reemplazada;
}

t_pagina* reemplazar_clock_modificado(t_proceso* proceso, t_pagina* nueva) {
    t_pagina* reemplazada = NULL;
    uint32_t cant_paginas = list_size(proceso->residentes);
    if (memoria_config.mostrar_algoritmo) log_info(logger, "Reemplazando por clock-m");
    // busco (u=0, m=0)
    for (int i = 0; i < cant_paginas; i++) {
        reemplazada = list_remove(proceso->residentes, 0);
        if (memoria_config.mostrar_algoritmo)
            log_info(logger, "Actual: pag %d (u=%d,m=%d)", reemplazada->numero, reemplazada->uso, reemplazada->modificado);
        if (reemplazada->uso == 0 && reemplazada->modificado == 0) {
            return reemplazada;
        }
        list_add(proceso->residentes, reemplazada);
    }
    // busco (u=0, m=1) y voy poniendo u en 0
    for (int i = 0; i < cant_paginas; i++) {
        reemplazada = list_remove(proceso->residentes, 0);
        if (memoria_config.mostrar_algoritmo)
            log_info(logger, "Actual: pag %d (u=%d,m=%d)", reemplazada->numero, reemplazada->uso, reemplazada->modificado);
        if (reemplazada->uso == 0 && reemplazada->modificado == 1) {
            return reemplazada;
        }
        reemplazada->uso = 0;
        list_add(proceso->residentes, reemplazada);
    }
    // si no la encontre, vuelvo al paso 1
    return reemplazar_clock_modificado(proceso, nueva);
}

uint32_t reemplazar_pagina(t_proceso* proceso, t_pagina* nueva) {
    t_pagina* reemplazada =  memoria_config.algoritmo_reemplazo == CLOCK
        ? reemplazar_clock(proceso, nueva)
        : reemplazar_clock_modificado(proceso, nueva);
    if (memoria_config.mostrar_algoritmo)
        log_info(logger, "Reemplazo la pag %d", reemplazada->numero);
    reemplazada->presencia = 0;
    if (reemplazada->modificado == 1) {
        usleep(memoria_config.retardo_swap * 1000);
        swap_escribir_pagina(proceso, reemplazada->numero, 
            memoria_principal + reemplazada->marco * memoria_config.tamanio_pagina);
        reemplazada->modificado = 0;
    }
    return reemplazada->marco;
}

uint32_t insertar_pagina(t_proceso* proceso, t_pagina* pagina) {
    uint32_t marco = list_size(proceso->residentes) < memoria_config.marcos_por_proceso ?
        proximo_marco_libre(proceso) : reemplazar_pagina(proceso, pagina);
    pagina->marco = marco;
    if (memoria_config.mostrar_algoritmo) 
        log_info(logger, "pagina insertada en el marco %d", marco);
    list_add(proceso->residentes, pagina);
    return marco;
}

uint32_t traer_a_memoria(uint32_t tabla_1er_nivel, t_pagina* pagina) {
    pagina->uso = 1;
    if (pagina->presencia) return pagina->marco;
    pagina->modificado = 0;
    pagina->presencia = 1;
    // Busco el proceso
    t_proceso* proceso = list_get(primer_nivel, tabla_1er_nivel);
    // Busco un marco libre (reemplazando una pag si hace falta)
    uint32_t marco = insertar_pagina(proceso, pagina);
    // Pongo en uno el bit
    bitarray_set_bit(marcos_libres, marco);
    // Copio la pagina desde la swap
    void* contenido = swap_leer_pagina(proceso, pagina->numero);
    usleep(memoria_config.retardo_swap * 1000);
    pthread_mutex_lock(&mutex_memoria);
    memcpy(memoria_principal + marco * memoria_config.tamanio_pagina, contenido, memoria_config.tamanio_pagina);
    pthread_mutex_unlock(&mutex_memoria);
    free(contenido);
    // Devuelvo el marco
    return marco;
}

