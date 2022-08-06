#include "tlb.h"

t_list* tlb;

void inicializar_tlb() {
    tlb = list_create();
}

void vaciar_tlb() {
    list_clean_and_destroy_elements(tlb, free);
}

void sacar_paginas_con_marco(uint32_t marco) {
    for (int i = 0; i < list_size(tlb); i++) {
        uint32_t* entrada = (uint32_t*) list_get(tlb, i);
        uint32_t marco_entrada = entrada[1];
        if (marco_entrada == marco) {
            free(list_remove(tlb, i));
            return;
        }
    }
}

void agregar_a_tlb(uint32_t pagina, uint32_t marco) {
    sacar_paginas_con_marco(marco);
    uint32_t* entrada = malloc(2 * sizeof(uint32_t));
    entrada[0] = pagina;
    entrada[1] = marco;
    if (list_size(tlb) == cpu_config->entradas_tlb)
        free(list_remove(tlb, 0));
    list_add(tlb, entrada);
}

uint32_t buscar_en_tlb(uint32_t pagina) {
    for (int i = 0; i < list_size(tlb); i++) {
        uint32_t* entrada = (uint32_t*) list_get(tlb, i);
        if (entrada[0] == pagina) {
            if (cpu_config->algoritmo_reemplazo_tlb == LRU)
                list_add(tlb, list_remove(tlb, i));
            log_info(logger, "TLB HIT: pagina %d -> marco %d", pagina, entrada[1]);
            return entrada[1];
        } 
    }
    log_info(logger, "TLB MISS: pagina %d", pagina);
    return -1;
}
