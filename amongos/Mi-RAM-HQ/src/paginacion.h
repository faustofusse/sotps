#ifndef PAGINACION_H_
#define PAGINACION_H_

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include "constantes.h"

typedef struct {
     u_int32_t frame; // -1 si fue eliminada
     u_int32_t presencia;
     u_int32_t uso;
} t_pagina;

typedef struct {
     u_int32_t pid;
     u_int32_t tamanio;
     t_list* paginas;
     t_list* tcbs; // Lista de tcbs no eliminados
} p_proceso;

t_bitarray* frames_real; // Me dice cuales estan ocupados y cuales libres
t_bitarray* frames_swap; // Lo mismo pero de la memoria virtual

t_list* procesos; // Lista de procesos
t_list* paginas_memoria; // Lista de pags que estan en memoria real

u_int32_t puntero; // Para el clock

int algoritmo_reemplazo, tamanio_pag, tamanio_swap, tamanio_memoria;

void* memoria_swap;

void pag_iniciar_variables(void* memoria_principal, int tamanio_memoria, int algoritmo_reemplazo, char* path_swap, int tamanio_pag, int tamanio_swap);

bool pag_guardar_patota(u_int32_t pid, char* tareas, int cantidad_tripulantes, char* posiciones, int caracter);
char* pag_siguiente_tarea(u_int32_t pid, u_int32_t tid);
void pag_cambiar_estado(u_int32_t pid, u_int32_t tid, char estado);
void pag_mover(u_int32_t pid, u_int32_t tid, u_int32_t x, u_int32_t y);
void pag_eliminar_pcb(u_int32_t pid);
void pag_eliminar_tcb(u_int32_t pid, u_int32_t tid);
void pag_dump(bool swap);

void pag_mostrar_memoria();

#endif