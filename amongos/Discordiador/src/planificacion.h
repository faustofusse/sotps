#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#define FIFO 1
#define RR 2

#define WAIT 0
#define POST 1

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <constantes.h>
#include <semaphore.h>
#include <paquete.h>

extern t_log* logger;

typedef struct {
	char* nombre;
	t_list* parametros;
	int tiempo;
	int x;
	int y;
	bool io; // indica si la tarea es de entrada/salida
} t_tarea;

typedef struct {
	pthread_t hilo;
	sem_t semaforo, mutex, sem_destruir, pausado;
	int estado;
	int x, y, tid, pid;
	int tiempo_en_cpu;
	int ram, store; // sockets
	t_tarea* tarea;
	bool termino;
	char caracter;
} t_tripulante;

t_queue* cola_exec; // Para los sabotajes
t_queue* cola_ready; // Para los sabotajes

t_list* colas[5];
sem_t sem_colas[5];

sem_t contador_ready, sem_planificacion, dispositivos_io, cpus_libres;

pthread_t hilo_planificador;

int grado_multitarea, retardo_cpu, quantum, duracion_sabotaje, algoritmo;

sem_t mutex_pausa, mutex_emergencia;
bool pausa, planificando, emergencia;

bool consumio_el_quantum(t_tripulante* t);
void* planificar(void* arg);
void iniciar_planificacion();
void pausar_planificacion();
void cambiar_estado(t_tripulante* tripulante, int estado);
void sacar_tripulante(t_tripulante* tripulante, t_list* lista);
void rafaga_cpu(t_tripulante* t);
void rafaga_io(t_tripulante* t, bool ultima);
void destruir_tarea(t_tarea* tarea);
void inicializar_variables_planificacion(t_config* config);
void avisar_a_ram(t_tripulante* tripulante, int estado);

#endif