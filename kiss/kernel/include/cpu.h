#ifndef KERNEL_CPU_H_
#define KERNEL_CPU_H_

#include "config.h"

typedef struct {
     int conexion;
     uint32_t bloqueo_restante;
     uint32_t tiempo_bloqueado;
     uint64_t timestamp;
     bool suspendido;
     t_pcb* pcb;
} t_proceso;


extern int conexion_dispatch, conexion_interrupt;

void conectar_cpu();
void enviar_interrupcion();
void enviar_proceso(t_pcb* pcb);
int recibir_pcb(t_proceso* p);

#endif
