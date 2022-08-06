#ifndef TYPES_H
#define TYPES_H
#define METADATA_SIZE 9

#include <stdint.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/config.h>

void* memoria_principal;
t_bitarray* bitmap_memoria;
t_bitarray* bitmap_procesos;

int tamanio_memoria, tamanio_pagina, tipo_asignacion, marcos_por_carpincho, algoritmo_mmu;
t_log* logger;

t_list* paginas; // lista de paginas en memoria real
t_list* carpinchos; // lista de procesos

typedef struct {
     uint32_t pid;
     uint32_t numero;
     uint32_t frame;
     uint8_t presencia;
     // clock:
     uint8_t uso;
     uint8_t modificado;
} t_pagina;

typedef struct {
     uint32_t prevAlloc;
     uint32_t nextAlloc;
     uint8_t isFree;
} HeapMetadata;

typedef struct {
     uint32_t pid;
     uint32_t hits, misses;
     t_list* paginas;
     t_list* residentes;
     uint32_t primer_marco;
} t_carpincho;

typedef int32_t mate_pointer;

#endif