#include "operaciones.h"

pthread_mutex_t mutex_tablas, mutex_memoria, mutex_swap;
t_bitarray* marcos_libres;

void memoria_inicializar() {
    memoria_principal = malloc(memoria_config.tamanio_memoria);
    memset(memoria_principal, 0, memoria_config.tamanio_memoria);
    primer_nivel = list_create();
    segundo_nivel = list_create();

    // bitarray
    uint32_t cantidad_marcos = memoria_config.tamanio_memoria / memoria_config.tamanio_pagina;
    uint32_t tamanio_bitarray = cantidad_marcos / 8;
    void* bits = malloc(tamanio_bitarray);
    marcos_libres = bitarray_create(bits, cantidad_marcos);
    for (int i = 0; i < cantidad_marcos; i++) bitarray_clean_bit(marcos_libres, i);

    pthread_mutex_init(&mutex_tablas, NULL);
    pthread_mutex_init(&mutex_memoria, NULL);
    pthread_mutex_init(&mutex_swap, NULL);
}

uint32_t memoria_leer(uint32_t direccion_fisica) {
    uint32_t valor = 0;
    pthread_mutex_lock(&mutex_memoria);
    memcpy(&valor, memoria_principal + direccion_fisica, sizeof(uint32_t));
    pthread_mutex_unlock(&mutex_memoria);
    return valor;
}

bool memoria_escribir(uint32_t direccion_fisica, uint32_t valor) {
    pthread_mutex_lock(&mutex_memoria);
    memcpy(memoria_principal + direccion_fisica, &valor, sizeof(uint32_t));
    pthread_mutex_unlock(&mutex_memoria);
    pthread_mutex_lock(&mutex_tablas);
    t_pagina* pagina = buscar_pagina(direccion_fisica);
    pagina->modificado = 1;
    pthread_mutex_unlock(&mutex_tablas);
    return true;
}

uint32_t memoria_pedir_tabla(uint32_t tabla_1er_nivel, uint32_t pagina) {
    pthread_mutex_lock(&mutex_tablas);
    if (tabla_1er_nivel >= list_size(primer_nivel) || pagina > memoria_config.entradas_por_tabla) {
        log_error(logger, "Valores invalidos al pedir la pagina %d de la tabla %d", pagina, tabla_1er_nivel);
        return -1;
    }
    t_proceso* proceso = (t_proceso*) list_get(primer_nivel, tabla_1er_nivel);
    uint32_t tabla_2do_nivel = proceso->tablas[pagina];
    pthread_mutex_unlock(&mutex_tablas);
    return tabla_2do_nivel;
}

uint32_t memoria_pedir_marco(uint32_t tabla_1er_nivel, uint32_t tabla_2do_nivel, uint32_t numero_pagina) {
    pthread_mutex_lock(&mutex_tablas);
    if (tabla_2do_nivel >= list_size(segundo_nivel) || numero_pagina > memoria_config.entradas_por_tabla) {
        log_error(logger, "Valores invalidos al pedir la pagina %d de la tabla %d", numero_pagina, tabla_2do_nivel);
        return -1;
    }
    t_pagina** tabla = (t_pagina**) list_get(segundo_nivel, tabla_2do_nivel);
    t_pagina* pagina = tabla[numero_pagina];
    uint32_t marco = traer_a_memoria(tabla_1er_nivel, pagina);
    pthread_mutex_unlock(&mutex_tablas);
    return marco;
}

