#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <socket.h>
#include <paquete.h>
#include <constantes.h>
#include <commons/log.h>
#include <commons/config.h>

int socket_memoria;
t_log* logger;
t_config* config;

int multiprogramacion, multiprocesamiento, retardo_cpu, 
algoritmo_planificacion, estimacion_inicial, tiempo_deadlock;
double alfa;

typedef struct {
     char* nombre;
     int duracion;
} t_io;

t_list* dispositivos_io;

typedef struct {
     t_estado estado;
} t_carpincho;

t_config* crear_config();
t_log* crear_logger();
void recibir(int, int);
int conectar_memoria(t_config* config);
void obtener_valores_config(t_config* c);

#endif