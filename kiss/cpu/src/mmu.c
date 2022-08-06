#include "mmu.h"

t_pcb* pcb;
uint32_t entradas_por_tabla, tamanio_pagina;
int conexion_memoria;

void memoria_write(uint32_t direccion_logica, uint32_t valor) {
    uint32_t direccion_fisica = traducir(direccion_logica);
    t_paquete* paquete = crear_paquete(MEMORIA_ESCRIBIR);
    agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
    agregar_a_paquete(paquete, &valor, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    uint32_t resultado = recibir_numero(conexion_memoria);
    if (resultado == ESCRITURA_OK)
        log_info(logger, "WRITE (logica = %d) <- %d", direccion_logica, valor);
        /* log_info(logger, "WRITE (logica = %d, fisica = %d) <- %d", direccion_logica, direccion_fisica, valor); */
    else if (resultado == ESCRITURA_ERROR) 
        log_error(logger, "Error al escribir el valor %d en la direccion fisica %d", valor, direccion_fisica);
    else
        log_error(logger, "Se recibio una respuesta rara al escribir en memoria");
}

uint32_t memoria_read(uint32_t direccion_logica) {
    uint32_t direccion_fisica = traducir(direccion_logica);
    t_paquete* paquete = crear_paquete(MEMORIA_LEER);
    agregar_a_paquete(paquete, &direccion_fisica, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    uint32_t leido = recibir_numero(conexion_memoria);
    log_info(logger, "READ (logica = %d) -> %d", direccion_logica, leido);
    /* log_info(logger, "READ (logica = %d, fisica = %d) -> %d", direccion_logica, direccion_fisica, leido); */
    return leido;
}

void conectar_memoria() {
    conexion_memoria = crear_conexion(cpu_config->ip_memoria, cpu_config->puerto_memoria);
    enviar_numero(conexion_memoria, HANDSHAKE_CPU);
    int numero = recibir_numero(conexion_memoria);
    if (numero != HANDSHAKE_MEMORIA) {
        log_error(logger, "Handshake con memoria fallo. Reintentando...");
        conectar_memoria();
        return;
    }
    t_list* valores = recibir_paquete(conexion_memoria);
    tamanio_pagina = *((uint32_t*) list_get(valores, 0));
    entradas_por_tabla = *((uint32_t*) list_get(valores, 1));
    list_destroy_and_destroy_elements(valores, free);
    log_info(logger, "Se conecto la memoria");
}

uint32_t pedir_a_memoria(uint32_t operacion, uint32_t a, uint32_t b) {
    t_paquete* paquete = crear_paquete(operacion);
    agregar_a_paquete(paquete, &a, sizeof(uint32_t));
    agregar_a_paquete(paquete, &b, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    return recibir_numero(conexion_memoria);
}

uint32_t pedir_tabla_2do_nivel(uint32_t tabla_1er_nivel, uint32_t pagina) {
    uint32_t tabla_2do_nivel = pedir_a_memoria(PEDIR_TABLA, tabla_1er_nivel, pagina);
    if (tabla_2do_nivel == -1) log_error(logger, "Error al pedir la tabla de 2do nivel");
    return tabla_2do_nivel;
}

uint32_t pedir_marco(uint32_t tabla_1er_nivel, uint32_t tabla_2do_nivel, uint32_t pagina) {
    /* uint32_t marco = pedir_a_memoria(PEDIR_MARCO, tabla_2do_nivel, pagina); */
    t_paquete* paquete = crear_paquete(PEDIR_MARCO);
    agregar_a_paquete(paquete, &tabla_1er_nivel, sizeof(uint32_t));
    agregar_a_paquete(paquete, &tabla_2do_nivel, sizeof(uint32_t));
    agregar_a_paquete(paquete, &pagina, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    uint32_t marco = recibir_numero(conexion_memoria);
    if (marco == -1) log_error(logger, "Error al pedir el marco");
    return marco;
}

uint32_t traducir(uint32_t direccion_logica) {
    uint32_t numero_pagina = floor((float) direccion_logica / (float) tamanio_pagina);
    uint32_t desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
    uint32_t marco = buscar_en_tlb(numero_pagina);
    if (marco != -1) return marco * tamanio_pagina + desplazamiento;
    uint32_t tabla_1er_nivel = pcb->tabla_paginas;
    uint32_t entrada_tabla_1er_nivel = floor((float) numero_pagina / (float) entradas_por_tabla);
    uint32_t tabla_2do_nivel = pedir_tabla_2do_nivel(tabla_1er_nivel, entrada_tabla_1er_nivel);
    uint32_t entrada_tabla_2do_nivel = numero_pagina % entradas_por_tabla;
    marco = pedir_marco(tabla_1er_nivel, tabla_2do_nivel, entrada_tabla_2do_nivel);
    agregar_a_tlb(numero_pagina, marco);
    return marco * tamanio_pagina + desplazamiento;
}

