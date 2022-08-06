#include "planificacion.h"

bool consumio_el_quantum(t_tripulante* t){
	return algoritmo == RR && t->tiempo_en_cpu >= quantum;
}

void rafaga_cpu(t_tripulante* t) {
	sleep(retardo_cpu);
	t->tiempo_en_cpu ++;
	sem_post(&t->semaforo);
}

void rafaga_io(t_tripulante* t, bool ultima) {
	sleep(retardo_cpu);
	if (ultima) sem_post(&dispositivos_io); 
	else sem_post(&t->semaforo);
}

void pasar_uno_a_exec() {
	sem_wait(&sem_colas[READY]);
	// Me fijo si esta vacia (pasa cuando se expulsa un tripulante)
	if (list_size(colas[READY]) == 0) {
		sem_post(&sem_colas[READY]);
		return;
	}
	// Busco el primero de la lista
	t_tripulante* t = list_get(colas[READY], 0);
	sem_post(&sem_colas[READY]);
	// Cambio el estado
	cambiar_estado(t, EXEC);
}

void pasar_los_nuevos_a_ready() {
	sem_wait(&sem_colas[NEW]);
	int cantidad = list_size(colas[NEW]);
	sem_post(&sem_colas[NEW]);
	for (int i = 0; i < cantidad; i++) 
		cambiar_estado((t_tripulante*) list_get(colas[NEW], 0), READY);
}

void* planificar(void* arg) {
	while (1) {
		// Espero que haya algun CPU disponible
		sem_wait(&cpus_libres);
		// Espero que haya alguien en la cola de ready
		sem_wait(&contador_ready);
		// Espero a que se active la planificacion
		sem_wait(&sem_planificacion);
		// Paso uno de ready a exec
		pasar_uno_a_exec();
		// Le digo que siga planificando nomas
		sem_post(&sem_planificacion);
	}
}

void todos_los_semaforos(int valor)
{
	for (int i = 0; i <= EXIT; i++) sem_wait(&sem_colas[i]);
	for (int i = 0; i <= EXIT; i++) {
		for (int j = 0; j < list_size(colas[i]); j++) {
			t_tripulante* t = list_get(colas[i], j);
			if (valor == WAIT) sem_wait(&t->pausado);
			if (valor == POST) sem_post(&t->pausado);
		}
	}
	for (int i = 0; i <= EXIT; i++) sem_post(&sem_colas[i]);
}

void iniciar_planificacion() 
{
	sem_wait(&mutex_pausa);
	if (pausa) {
		pasar_los_nuevos_a_ready();
		pausa = false;
		if (planificando) todos_los_semaforos(POST);
		planificando = true;
		sem_post(&sem_planificacion);
	}
	sem_post(&mutex_pausa);
}

void pausar_planificacion()
{
	sem_wait(&mutex_pausa);
	if (!pausa && planificando) {
		pausa = true;
		sem_wait(&sem_planificacion);
		todos_los_semaforos(WAIT);
	}
	sem_post(&mutex_pausa);
}

void sacar_tripulante(t_tripulante* tripulante, t_list* lista)
{
	t_tripulante* t;
	for (int i = 0; i<list_size(lista); i++){
		t = list_get(lista, i);
		if (t->tid == tripulante->tid && t->pid == tripulante->pid){
			list_remove(lista, i);
			break;
		}
	}
}

void avisar_a_ram(t_tripulante* tripulante, int estado)
{
	t_paquete* paquete = crear_paquete(estado == EXIT ? EXPULSAR_TRIPULANTE : CAMBIAR_ESTADO);
	if (estado != EXIT) agregar_a_paquete(paquete, (void*) &(char_estados[estado]), sizeof(char));
	agregar_a_paquete(paquete, &tripulante->tid, sizeof(int));
	agregar_a_paquete(paquete, &tripulante->pid, sizeof(int));
	enviar_paquete(paquete, tripulante->ram);
	eliminar_paquete(paquete);
}

void cambiar_estado(t_tripulante* tripulante, int nuevo)
{
	// Espero al tripulante
	sem_wait(&tripulante->mutex);
	// Si esta en exit, no puede cambiar de estado
	if (tripulante->estado == EXIT) {
		sem_post(&tripulante->mutex);
		return;
	}
	// Le aviso a la RAM
	avisar_a_ram(tripulante, nuevo);
	// Libero recursos
	if (tripulante->estado == EXEC) sem_post(&cpus_libres);
	if (tripulante->estado == BLOCKED) tripulante->tiempo_en_cpu = 0;
	// Lo saco de la cola en la que estaba
	sem_wait(&sem_colas[tripulante->estado]);
	sacar_tripulante(tripulante, colas[tripulante->estado]);
	sem_post(&sem_colas[tripulante->estado]);
	// Actualizo el estado
	tripulante->estado = nuevo;
	// Lo agrego a la cola del nuevo estado
	sem_wait(&sem_colas[nuevo]);
	list_add(colas[nuevo], tripulante);
	sem_post(&sem_colas[nuevo]);
	// Hago cosas en base al nuevo estado
	if (nuevo == READY) sem_post(&contador_ready);
	if (nuevo == EXIT) tripulante->termino = true;
	if (nuevo == EXEC || nuevo == EXIT) sem_post(&tripulante->semaforo);
	// Libero al tripulante
	sem_post(&tripulante->mutex);
}

void destruir_tarea(t_tarea* tarea)
{
	free(tarea->nombre);
	free(tarea);
}

void inicializar_variables_planificacion(t_config* config)
{
	pausa = true;
	planificando = false;
	emergencia = false;
	// Config
	grado_multitarea = config_get_int_value(config, "GRADO_MULTITAREA");
	retardo_cpu = config_get_int_value(config, "RETARDO_CICLO_CPU");
	quantum = config_get_int_value(config, "QUANTUM");
	duracion_sabotaje = config_get_int_value(config, "DURACION_SABOTAJE");
	algoritmo = strcmp(config_get_string_value(config, "ALGORITMO"), "RR") == 0 ? RR : FIFO;
	// Semaforos
	sem_init(&sem_planificacion, 0, 0);
	sem_init(&cpus_libres, 0, grado_multitarea);
	sem_init(&dispositivos_io, 0, 1);
	sem_init(&contador_ready, 0, 0);
	sem_init(&mutex_pausa, 0, 1);
	sem_init(&mutex_emergencia, 0, 1);
	// Colas
	for (int i = 0; i <= EXIT; i++) {
		colas[i] = list_create();
		sem_init(&(sem_colas[i]), 0, 1);
	}
	cola_exec = queue_create();
	cola_ready = queue_create();
	// Hilo
	pthread_create(&hilo_planificador, NULL, *planificar, NULL);
	pthread_detach(hilo_planificador);
}
