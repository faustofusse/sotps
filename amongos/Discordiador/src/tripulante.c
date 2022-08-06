#include "tripulante.h"

int ultimo_id = 0;
int caracter = 65;

int proximo_caracter()
{
	if (caracter == 91) { 
		caracter = 97;
		return caracter;
	} else {
		caracter++;
		return caracter - 1;
	}
}

int generar_patota_id()
{
	ultimo_id++;
	return ultimo_id;
}

void iniciar_patota(int cantidad_tripulantes, char* nombre_archivo, t_list* posiciones)
{
	FILE* archivo = fopen(nombre_archivo, "r");
	if (archivo == NULL) { printf("%s\n", "Error: no se encontro el archivo."); return; }
	t_paquete* paquete = crear_paquete(INICIAR_PATOTA);
	// Preparo las variables a enviar
	char* separador = ",";
	char* posiciones_string = lista_a_string(posiciones, separador);
	int pid = generar_patota_id();
	// Las agrego al paquete y lo envio
	agregar_a_paquete(paquete, (void*) &pid, sizeof(int));
	agregar_archivo_a_paquete(paquete, archivo, separador);
	agregar_a_paquete(paquete, (void*) &cantidad_tripulantes, sizeof(int));
	agregar_a_paquete(paquete, (void*) posiciones_string, strlen(posiciones_string) + 1);
	agregar_a_paquete(paquete, (void*) &caracter, sizeof(int));
	enviar_paquete(paquete, socket_ram);
	// Recibo confirmacion de que se creo la patota en la RAM
	int operacion = recibir_operacion(socket_ram);
	if (operacion == INICIAR_PATOTA) {
		// Inicio los tripulantes
		int cantidad_posiciones = list_size(posiciones);
		for (int i = 0; i < cantidad_tripulantes; i++) {
			int x = i > cantidad_posiciones - 1 ? 0 : atoi((char*) list_get(posiciones, i));
			int y = i > cantidad_posiciones - 1 ? 0 : atoi((char*) list_get(posiciones, i) + 2 * sizeof(char));
			iniciar_tripulante(pid, i + 1, x, y, (char) proximo_caracter());
		}
	} else {
		ultimo_id--;
		printf("Error al crear la patota!\n");
	}
	// Libero la memoria
	eliminar_paquete(paquete);
	fclose(archivo);
	free(posiciones_string);
	list_destroy(posiciones);
}

t_list* tripulantes_ordenados()
{
	t_list* tripulantes = list_create();
	t_tripulante* t;
	for (int i = 0; i <= EXIT; i++) sem_wait(&sem_colas[i]);
	for (int i = 0; i <= EXIT; i++) {
		for (int j = 0; j < list_size(colas[i]); j++) {
			t = (t_tripulante*) list_get(colas[i], j);
			for (int b = 0; b < list_size(tripulantes); b++) {
				if (t->caracter <( (t_tripulante*) list_get(tripulantes, b))->caracter) {
					list_add_in_index(tripulantes, b, t);
					break;
				} else if (b == list_size(tripulantes) - 1) {
					list_add(tripulantes, t);
					break;
				}
			}
			if (list_size(tripulantes) == 0) list_add(tripulantes, t);
		}
	}
	for (int i = 0; i <= EXIT; i++) sem_post(&sem_colas[i]);
	return tripulantes;
}

void listar_tripulantes()
{
	printf("--------------------------------------------------------------------\n");
	char* tiempo = temporal_get_string_time("%d/%m/%y %H:%M:%S");
	printf("Estado de la nave: %s\n", tiempo);
	free(tiempo);
	t_list* tripulantes = tripulantes_ordenados();
	for (int i = 0; i < list_size(tripulantes); i++) {
		t_tripulante* t = (t_tripulante*) list_get(tripulantes, i);
		sem_wait(&t->mutex);
		int estado = t->estado;
		sem_post(&t->mutex);
		printf("Tripulante: %d\tPatota: %d\tStatus: %s\t Id: %c\n", t->tid, t->pid, string_estados[estado], t->caracter);
	}
	list_destroy(tripulantes);
	printf("--------------------------------------------------------------------\n");
}

t_tripulante* buscar_tripulante(int pid, int tid)
{
	t_list* tripulantes = tripulantes_ordenados();
	for (int i = 0; i < list_size(tripulantes); i++) {
		t_tripulante* t = (t_tripulante*) list_get(tripulantes, i);
		if (t->pid == pid && t->tid == tid) {
			list_destroy(tripulantes);
			return t;
		}
	}
	list_destroy(tripulantes);
	return NULL;
}

void obtener_bitacora(int tid, int pid)
{
	printf("Bitacora del tripulante %d(P%d)\n", tid, pid);
}

bool es_de_entrada_salida(char* nombre)
{
	for (int i = 0; i < sizeof(tareas_io)/sizeof(char*); i++)
		if (strcmp(nombre, tareas_io[i]) == 0)
			return true;
	return false;
}

t_tarea* crear_tarea(char* nombre, t_list* parametros, int x, int y, int tiempo)
{
	t_tarea* tarea = malloc(sizeof(t_tarea));
	tarea->nombre = nombre;
	tarea->parametros = parametros;
	tarea->tiempo = tiempo;
	tarea->x = x;
	tarea->y = y;
	tarea->io = es_de_entrada_salida(nombre);
	return tarea;
}

t_tarea* pedir_tarea(t_tripulante* tripulante)
{
	// Si ya tiene una tarea, la destruyo
	if (tripulante->tarea != NULL) destruir_tarea(tripulante->tarea);
	// Pido a la RAM la proxima tarea
	t_paquete* paquete = crear_paquete(PROXIMA_TAREA);
	agregar_a_paquete(paquete, &tripulante->tid, sizeof(int));
	agregar_a_paquete(paquete, &tripulante->pid, sizeof(int));
	enviar_paquete(paquete, tripulante->ram);
	eliminar_paquete(paquete);
	// Espero la respuesta
	int operacion = recibir_operacion(tripulante->ram);
	if (operacion == FIN_TAREAS || operacion != PROXIMA_TAREA) return NULL;
	// Separo el string de la tarea
	t_list* valores = recibir_paquete(tripulante->ram);
	char* tarea_string = (char*) list_get(valores, 0);
	separar_string(valores, tarea_string, ";");
	// Separo el primer elemento (que tiene el nombre y los parametros)
	t_list* parametros = list_create();
	separar_string(parametros, list_get(valores, 0), " ");
	list_remove(valores, 0);
	// Saco el nombre de los parametros
	char* nombre = (char*) list_get(parametros, 0);
	list_remove(parametros, 0);
	// Creo la tarea
	t_tarea* tarea = crear_tarea( nombre, parametros,
		atoi((char*) list_get(valores, 0)), // x
		atoi((char*) list_get(valores, 1)), // y
		atoi((char*) list_get(valores, 2)) // tiempo
	);
	// Libero memoria
	list_destroy(parametros);
	list_destroy(valores);
	return tarea;
}

void iniciar_tripulante(int pid, int tid, int x, int y, char caracter)
{
	// Reservo memoria y inicializo variables
	t_tripulante* t = malloc(sizeof(t_tripulante));
	t->estado = NEW;
	t->pid = pid;
	t->tid = tid;
	t->x = x;
	t->y = y;
	t->tarea = NULL;
	t->termino = false;
	t->caracter = caracter;
	// Conectarse a los servidores
	t->store = conectarse_a_store();
	t->ram = conectarse_a_ram();
	// Planificacion
	t->tiempo_en_cpu = 0;
	sem_init(&t->semaforo, 0, 0);
	sem_init(&t->sem_destruir, 0, 0);
	sem_init(&t->mutex, 0, 1);
	sem_init(&t->pausado, 0, 1);
	// Pido la primer tarea
	t->tarea = pedir_tarea(t);
	// Creo el hilo
	pthread_create(&t->hilo, NULL, *hacer_tareas, (void*) t);
	pthread_detach(t->hilo);
	// Si no esta pausada la planificacion, lo paso a ready
	sem_wait(&mutex_pausa);
	cambiar_estado(t, pausa ? NEW : READY);
	sem_post(&mutex_pausa);
}

void destruir_tripulante(t_tripulante* t)
{
	if (t->tarea != NULL) destruir_tarea(t->tarea);
	free(t);
}

void termino_el_proceso(int pid)
{
	// Se fija si todos los tripulantes estan en exit
	bool termino = true;
	t_list* tripulantes = tripulantes_ordenados();
	for (int i = 0; i < list_size(tripulantes); i++) {
		t_tripulante* t = (t_tripulante*) list_get(tripulantes, i);
		sem_wait(&t->mutex);
		int estado = t->estado;
		int proceso = t->pid;
		sem_post(&t->mutex);
		if (proceso == pid && estado != EXIT) {
			termino = false;
			break;
		}
	}
	// Si efectivamente termino, destruyo todos los tripulantes
	if (termino) {
		for (int i = 0; i < list_size(tripulantes); i++) {
			t_tripulante* t = (t_tripulante*) list_get(tripulantes, i);
			if (t->pid == pid) {
				sem_wait(&sem_colas[EXIT]);
				sacar_tripulante(t, colas[EXIT]);
				sem_post(&sem_colas[EXIT]);
				sem_post(&t->sem_destruir);
			}
		}
	}
	list_destroy(tripulantes);
}

void expulsar_tripulante(int tid, int pid)
{
	t_tripulante* tripulante = buscar_tripulante(pid, tid);
	if (tripulante == NULL) printf("Error: no existe el tripulante.\n"); 
	else if (tripulante->estado == EXIT) printf("Error: el tripulante ya esta en la cola de salida.\n");
	else {
		sem_wait(&tripulante->mutex);
		tripulante->termino = true;
		sem_post(&tripulante->mutex);
		sem_wait(&mutex_pausa);
		if (pausa) sem_post(&tripulante->pausado);
		sem_post(&mutex_pausa);
		sem_post(&tripulante->semaforo);
	}
}

bool esta_en_la_tarea(t_tripulante* t) {
	return ( t->tarea->x == t->x && t->tarea->y == t->y );
}

void* hacer_tareas(void* arg) 
{
	t_tripulante* t = (t_tripulante*) arg;
	unsigned tiempo_haciendo_tarea = 0;
	bool haciendo_io = false; // Me dice si pedi o no la E/S
	while (t->tarea != NULL) {
		sem_wait(&t->pausado);
		sem_post(&t->pausado);

		sem_wait(&t->semaforo);

		// Si el tripulante termino, salgo del while
		sem_wait(&t->mutex);
		if (t->termino && t->estado == READY) sem_wait(&contador_ready);
		bool salir = t->termino || t->estado == EXIT;
		sem_post(&t->mutex);
		if (salir) break;

		// Si no termino, me fijo si ...
		if (consumio_el_quantum(t) && !haciendo_io) {

			// Consumio todo su quantum (si es FIFO me se saltea porque devuelve false)
			t->tiempo_en_cpu = 0;
			cambiar_estado(t, READY);

		} else if (esta_en_la_tarea(t) && t->tarea->io && !haciendo_io) {

			// Esta en una tarea de E/S pero no la empezo => hago una rafaga para pedir la E/S
			rafaga_cpu(t);
			haciendo_io = true;
			// Bloqueo el tripulante y Espero a que se libere un dispositivo de E/S
			cambiar_estado(t, BLOCKED);
			sem_wait(&dispositivos_io);

		} else if (esta_en_la_tarea(t)) { 

			// Esta en una tarea (normal o E/S)
			tiempo_haciendo_tarea++;
			bool ultima = tiempo_haciendo_tarea >= t->tarea->tiempo;
			if (haciendo_io) rafaga_io(t, ultima); else rafaga_cpu(t);
			// Si no es la ultima rafaga, sigo pa lante noma
			if (!ultima) continue;
			// Pido la proxima tarea (si no fue eliminado)
			t->tarea = pedir_tarea(t);
			tiempo_haciendo_tarea = 0;
			// Si estaba haciendo E/S, libero el dispositivo y lo paso a ready
			if (haciendo_io) {
				haciendo_io = false;
				cambiar_estado(t, t->tarea == NULL ? EXIT : READY);
			}

		} else {

			// No esta en la tarea
			mover_hacia(t, t->tarea->x, t->tarea->y);
			rafaga_cpu(t);

		}
	}
	cambiar_estado(t, EXIT);
	termino_el_proceso(t->pid);
	close(t->ram);
	close(t->store);
	sem_wait(&t->sem_destruir);
	destruir_tripulante(t);
	return NULL;
}