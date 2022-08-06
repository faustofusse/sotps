#ifndef PCB_H_
#define PCB_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <commons/collections/list.h>

typedef struct {
    uint32_t id;
    uint32_t tamanio;
    uint32_t program_counter;
    uint32_t tabla_paginas;
    uint32_t estimacion_rafaga;
    char* instrucciones;
} t_pcb;

uint32_t tamanio_pcb(t_pcb* pcb);
t_pcb* crear_pcb(uint32_t id, uint32_t tamanio, char* instrucciones, uint32_t estimacion_rafaga);
void destruir_pcb(t_pcb* pcb);
void* serializar_pcb(t_pcb* pcb);
t_pcb* deserializar_pcb(void* contenido);

#endif
