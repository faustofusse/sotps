#ifndef TRIPULANTE_H_
#define TRIPULANTE_H_

#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <socket.h>
#include <paquete.h>
#include <pthread.h>
#include <utils.h>
#include <constantes.h>
#include <stdbool.h>
#include <signal.h>

#include "sabotaje.h" // tripulante.h <- sabotaje.h <- planificacion.h
#include "Discordiador.h"

extern int socket_ram;

void iniciar_patota(int cantidad_tripulantes, char* nombre_archivo, t_list* posiciones);
void iniciar_tripulante(int pid, int tid, int x, int y, char caracter);
void listar_tripulantes();
void expulsar_tripulante(int tid, int pid);
void eliminar_tripulante(t_tripulante* t);
void obtener_bitacora(int tid, int pid);
void destruir_tripulante(t_tripulante* t);
void* hacer_tareas(void* arg);
t_tripulante* crear_tripulante(int pid, int tid, int x, int y, char caracter);
t_tarea* pedir_tarea(t_tripulante* tripulante);

#endif
