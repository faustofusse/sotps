#ifndef KERNEL_CONFIG_H_
#define KERNEL_CONFIG_H_

#include <stdint.h>
#include <utils.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <socket.h>
#include <paquete.h>
#include <pcb.h>
#include <constantes.h>
#include <instrucciones.h>

typedef struct {
     char* ip_memoria;
     int puerto_memoria;
     char* ip_cpu;
     int puerto_cpu_dispatch;
     int puerto_cpu_interrupt;
     int puerto_escucha;
     int algoritmo_planificacion;
     int estimacion_inicial;
     float alfa;
     int grado_multiprogramacion;
     int tiempo_maximo_bloqueo;
} t_kernel_config;

extern t_kernel_config* kernel_config;

extern t_log* logger;

void leer_config(char* path);

#endif
