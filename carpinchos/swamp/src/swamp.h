#ifndef SWAMP_H_
#define SWAMP_H_

#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <paquete.h>
#include <socket.h>
#include <constantes.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <stdbool.h>

int socket_memoria, asignacion, marcos_por_carpincho, retardo, tamanio_swap, tamanio_pagina;
t_list *archivos, *bitmaps;

typedef struct {
     int marco;
     bool libre;
} t_pagina;
typedef struct {
     int pid;
     int archivo;
     t_list *paginas;
} t_carpincho;
t_list* carpinchos;

t_log* logger;

t_config* crear_config();
t_log* crear_logger();
void recibir(int, int);

// temporal:
void pruebas();
void dump_swap();

#endif