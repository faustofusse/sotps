#include "sabotaje.h"

void mover_hacia(t_tripulante* t, int x, int y) 
{
	sem_wait(&t->mutex);
	// Actualizo las variables
	if (x < t->x) t->x--;
	else if (x > t->x) t->x++;
	else if (y < t->y) t->y--;
	else if (y > t->y) t->y++;
	// Le aviso a la RAM
	t_paquete* paquete = crear_paquete(MOVER_TRIPULANTE);
	agregar_a_paquete(paquete, (void*) &t->tid, sizeof(int));
	agregar_a_paquete(paquete, (void*) &t->pid, sizeof(int));
	agregar_a_paquete(paquete, (void*) &t->x, sizeof(int));
	agregar_a_paquete(paquete, (void*) &t->y, sizeof(int));
	enviar_paquete(paquete, t->ram);
	eliminar_paquete(paquete);
	sem_post(&t->mutex);
}

int distancia_entre_dos_puntos(int x1, int y1, int x2, int y2)
{
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

t_tripulante* tripulante_mas_cercano(t_list* tripulantes, int x, int y)
{
	t_tripulante* menor = NULL;
	int distancia = 0;
	for (int i = 0; i < list_size(tripulantes); i++) {
		t_tripulante* t = list_get(tripulantes, i);
		sem_wait(&t->mutex);
		int d = distancia_entre_dos_puntos(x, y, t->x, t->y);
		sem_post(&t->mutex);
		if (d < distancia || menor == NULL) {
			distancia = d;
			menor = t;
		}
	}
	return menor;
}

void cambiar_estado_soft(t_tripulante* t, int viejo, int nuevo)
{
	sem_wait(&sem_colas[viejo]);
	sacar_tripulante(t, colas[viejo]);
	sem_post(&sem_colas[viejo]);
	sem_wait(&sem_colas[nuevo]);
	list_add(colas[nuevo], t);
	sem_post(&sem_colas[nuevo]);
	avisar_a_ram(t, nuevo);
}

void resolver_sabotaje(t_tripulante* t, int x, int y)
{
	bool estaba_en_ready = t->estado == READY;
	// Lo paso a exec (si no estaba)
	if (estaba_en_ready) cambiar_estado_soft(t, READY, EXEC);
	// Me muevo hacia la ubicacion del sabotaje
	while (t->x != x || t->y != y) {
		mover_hacia(t, x, y);
		sleep(retardo_cpu);
	}
	// Lo paso a blocked
	cambiar_estado_soft(t, EXEC, BLOCKED);
	// Resuelvo el sabotaje
	for (int i = 0; i < duracion_sabotaje; i++) sleep(retardo_cpu);
	// Lo meto al final de la cola de ready/exec
	queue_push(estaba_en_ready ? cola_ready : cola_exec, t);
}

void sabotaje(int x, int y)
{
	if (emergencia) return;
	sem_wait(&mutex_emergencia);
	emergencia = true;
	sem_post(&mutex_emergencia);
	bool* estaba_pausado = malloc(sizeof(bool));
	sem_wait(&mutex_pausa);
	*estaba_pausado = pausa;
	sem_post(&mutex_pausa);
	if (!(*estaba_pausado)) pausar_planificacion();
	// Busco el tripulante mas cercano (entre los que estan en ready/exec)
	t_list* tripulantes = list_create();
	sem_wait(&sem_colas[EXEC]);
	list_add_all(tripulantes, colas[EXEC]);
	sem_post(&sem_colas[EXEC]);
	sem_wait(&sem_colas[READY]);
	list_add_all(tripulantes, colas[READY]);
	sem_post(&sem_colas[READY]);
	t_tripulante* tripulante = tripulante_mas_cercano(tripulantes, x, y);
	// Paso todos los otros a BLOCKED
	for (int i = 0; i < list_size(tripulantes); i++) {
		t_tripulante* t = list_get(tripulantes, i);
		if (t == tripulante) continue;
		queue_push(t->estado == READY ? cola_ready : cola_exec, t);
		cambiar_estado_soft(t, t->estado, BLOCKED);
	}
	list_destroy(tripulantes);
	// Le digo que resuelta el sabotaje
	if (tripulante != NULL) resolver_sabotaje(tripulante, x, y);
	// Vuelvo todo como estaba
	while(!queue_is_empty(cola_exec)) cambiar_estado_soft(queue_pop(cola_exec), BLOCKED, EXEC);
	while(!queue_is_empty(cola_ready)) cambiar_estado_soft(queue_pop(cola_ready), BLOCKED, READY);
	// Reanudar planificacion
	sem_wait(&mutex_emergencia);
	emergencia = false;
	sem_post(&mutex_emergencia);
	if (!(*estaba_pausado)) iniciar_planificacion();
	free(estaba_pausado);
}