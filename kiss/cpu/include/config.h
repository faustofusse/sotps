#ifndef CPU_CONFIG_H_
#define CPU_CONFIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include <instrucciones.h>
#include <pcb.h>
#include <socket.h>
#include <paquete.h>
#include <utils.h>
#include <constantes.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

typedef struct {
    int entradas_tlb;
    int algoritmo_reemplazo_tlb;
    int retardo_noop;
    char* ip_memoria;
    int puerto_memoria;
    int puerto_cpu_dispatch;
    int puerto_cpu_interrupt;
} t_cpu_config;

extern t_cpu_config* cpu_config;
extern t_log* logger;
extern t_list *instrucciones;
extern int conexion_dispatch;
extern bool desalojar;
extern pthread_mutex_t mutex_desalojar;

void leer_config(char* path);

#endif
