#ifndef MAPA_C_
#define MAPA_C_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <nivel-gui.h>
#include <tad_nivel.h>
#include <pthread.h>
#include <commons/string.h>

void mapa_iniciar();
void mapa_agregar_tripulante(int pid, int tid, int x, int y, char caracter);
void mapa_sacar_tripulante(int pid, int tid);
void mapa_mover_tripulante(int pid, int tid, int x, int y);
void mapa_cambiar_estado(int pid, int tid, char estado);

#endif