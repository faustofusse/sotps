#include "pcb.h"

t_pcb* crear_pcb(uint32_t id, uint32_t tamanio, char* instrucciones, uint32_t estimacion_inicial) {
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pcb->id = id;
    pcb->tamanio = tamanio;
    pcb->program_counter = 0;
    pcb->tabla_paginas = 0;
    pcb->estimacion_rafaga = estimacion_inicial;
    pcb->instrucciones = instrucciones;
    return pcb;
}

void destruir_pcb(t_pcb* pcb) {
    free(pcb->instrucciones);
    free(pcb);
}

uint32_t tamanio_pcb(t_pcb* pcb) {
    uint32_t size_instrucciones = strlen(pcb->instrucciones) + 1;
    return 6 * sizeof(uint32_t) + size_instrucciones;
}

void* serializar_pcb(t_pcb* pcb) {
    uint32_t size_instrucciones = strlen(pcb->instrucciones) + 1;
    void* contenido = malloc(6 * sizeof(uint32_t) + size_instrucciones);
    memcpy(contenido + 0 * sizeof(uint32_t), &pcb->id, sizeof(uint32_t));
    memcpy(contenido + 1 * sizeof(uint32_t), &pcb->tamanio, sizeof(uint32_t));
    memcpy(contenido + 2 * sizeof(uint32_t), &pcb->program_counter, sizeof(uint32_t));
    memcpy(contenido + 3 * sizeof(uint32_t), &pcb->tabla_paginas, sizeof(uint32_t));
    memcpy(contenido + 4 * sizeof(uint32_t), &pcb->estimacion_rafaga, sizeof(uint32_t));
    memcpy(contenido + 5 * sizeof(uint32_t), &size_instrucciones, sizeof(uint32_t));
    memcpy(contenido + 6 * sizeof(uint32_t), pcb->instrucciones, size_instrucciones);
    return contenido;
}

t_pcb* deserializar_pcb(void* contenido) {
    uint32_t size_instrucciones = 0;
    t_pcb* pcb = malloc(sizeof(t_pcb));
    memcpy(&pcb->id,                contenido + 0 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&pcb->tamanio,           contenido + 1 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&pcb->program_counter,   contenido + 2 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&pcb->tabla_paginas,     contenido + 3 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&pcb->estimacion_rafaga, contenido + 4 * sizeof(uint32_t), sizeof(uint32_t));
    memcpy(&size_instrucciones,     contenido + 5 * sizeof(uint32_t), sizeof(uint32_t));
    pcb->instrucciones = malloc(size_instrucciones);
    memcpy(pcb->instrucciones, contenido + 6 * sizeof(uint32_t), size_instrucciones);
    free(contenido);
    return pcb;
}
