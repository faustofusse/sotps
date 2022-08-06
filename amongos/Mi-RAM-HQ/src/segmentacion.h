#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_

#define FF 0
#define BF 1

#include "constantes.h"

typedef struct {
     u_int32_t inicio;
     u_int32_t tamanio;
} t_segmento;

typedef struct {
     u_int32_t pid;
     t_list* segmentos;
} t_proceso;

t_list* huecos; // Lista de t_segmento
t_list* procesos; // Lista de t_proceso

int algoritmo;

bool seg_guardar_patota(u_int32_t pid, char* tareas, int cantidad_tripulantes, char* posiciones, int caracter);
char* seg_siguiente_tarea(u_int32_t pid, u_int32_t tid);
void seg_iniciar_variables(void* memoria, u_int32_t tamanio, int alg);
void seg_cambiar_estado(u_int32_t pid, u_int32_t tid, char estado);
void seg_mover(u_int32_t pid, u_int32_t tid, u_int32_t x, u_int32_t y);
void seg_eliminar_tcb(u_int32_t pid, u_int32_t tid);
// void seg_eliminar_pcb(u_int32_t pid);
void compactacion();
void seg_dump();

#endif