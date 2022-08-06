#ifndef CONSTANTES_RAM_H_
#define CONSTANTES_RAM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <commons/log.h>
#include <constantes.h>
#include <utils.h>
#include <semaphore.h>

#include "mapa.h"

// Esquemas
#define PAGINACION 0
#define SEGMENTACION 1
// Criterios de seleccion
#define FF 0
#define BF 1
// Algoritmos de swap
#define LRU 0
#define CLOCK 1
// Mapa
#define MAPA_ACTIVADO 0

extern t_log* logger;

sem_t mutex_memoria;
int tamanio_memoria;
void* memoria_principal;

// Las variables de los TCB y PCB se guardan en este orden en la memoria:
typedef struct {
	u_int32_t pid;
	u_int32_t tareas;
} PCB;
typedef struct {
	u_int32_t tid;
	char estado;
	u_int32_t x;
	u_int32_t y;
	u_int32_t instruccion;
	u_int32_t PCB;
} TCB; 

#endif