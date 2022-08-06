#include "acciones.h"

void cambiar_estado(u_int32_t tid, u_int32_t pid, char estado)
{
	if (MAPA_ACTIVADO) mapa_cambiar_estado(tid, pid, estado);
	if (esquema == SEGMENTACION) seg_cambiar_estado(pid, tid, estado);
	if (esquema == PAGINACION) pag_cambiar_estado(pid, tid, estado);
}

void expulsar(u_int32_t pid, u_int32_t tid)
{
	if (MAPA_ACTIVADO) mapa_sacar_tripulante(pid, tid);
	if (esquema == SEGMENTACION) seg_eliminar_tcb(pid, tid);
	if (esquema == PAGINACION) pag_eliminar_tcb(pid, tid);
}

void mover(u_int32_t pid, u_int32_t tid, u_int32_t x, u_int32_t y)
{
	if (MAPA_ACTIVADO) mapa_mover_tripulante(pid, tid, x, y);
	if (esquema == SEGMENTACION) seg_mover(pid, tid, x, y);
	if (esquema == PAGINACION) pag_mover(pid, tid, x, y);
}

bool iniciar_patota(u_int32_t pid, char* tareas, int cantidad_tripulantes, char* posiciones, int caracter)
{
	if (esquema == SEGMENTACION) return seg_guardar_patota(pid, tareas, cantidad_tripulantes, posiciones, caracter);
	if (esquema == PAGINACION) return pag_guardar_patota(pid, tareas, cantidad_tripulantes, posiciones, caracter);
	return false;
}

void enviar_proxima_tarea(u_int32_t tid, u_int32_t pid, int cliente)
{
	char* tarea = esquema == SEGMENTACION ? seg_siguiente_tarea(pid, tid) : pag_siguiente_tarea(pid, tid);
	if (tarea != NULL) { 
		t_paquete* paquete = crear_paquete(PROXIMA_TAREA);
		agregar_a_paquete(paquete, tarea, strlen(tarea) + 1);
		enviar_paquete(paquete, cliente);
		eliminar_paquete(paquete);
	} else {
		enviar_numero(cliente, FIN_TAREAS);
	}
	free(tarea);
}

// ------ RECIBIR DEL DISCORDIADOR

void recibir(int cliente, int operacion)
{
	t_list* valores = NULL;
	u_int32_t tid, pid, x, y;

	switch (operacion) {
		case INICIAR_PATOTA:
			valores = recibir_paquete(cliente);
			// Obtener los valores
			pid = *((u_int32_t*) list_get(valores, 0));
			char* tareas = list_get(valores, 1);
			int tripulantes = *((int*) list_get(valores, 2));
			char* posiciones = list_get(valores, 3);
			int caracter = *((int*) list_get(valores, 4));
			// Iniciar la patota y los tripulantes
			bool creada = iniciar_patota(pid, tareas, tripulantes, posiciones, caracter);
			// Envio confirmacion al discordiador
			enviar_numero(cliente, creada ? INICIAR_PATOTA : -1); // hacer una op ERROR
			// Saco elementos de valores, para que no haga 'free' de algo ya liberado
			if (creada) list_remove(valores, 1); // saco las tareas
			break;
		case CAMBIAR_ESTADO:
			valores = recibir_paquete(cliente);
			// Obtener los valores
			char estado = *((char*) list_get(valores, 0));
			tid = *((u_int32_t*) list_get(valores, 1));
			pid = *((u_int32_t*) list_get(valores, 2));
			// Cambiar el estado
			cambiar_estado(tid, pid, estado);
			log_info(logger, "Tripulante %d (P%d) -> %c", tid, pid, estado);
			break;
		case EXPULSAR_TRIPULANTE:
			valores = recibir_paquete(cliente);
			// Obtener los valores
			tid = *((u_int32_t*) list_get(valores, 0));
			pid = *((u_int32_t*) list_get(valores, 1));
			// Expulsar tripulante
			expulsar(pid, tid);
			log_info(logger, "Finalizo tripulante %d (P%d)", tid, pid);
			break;
		case MOVER_TRIPULANTE:
			valores = recibir_paquete(cliente);
			// Obtener los valores
			tid = *((u_int32_t*) list_get(valores, 0));
			pid = *((u_int32_t*) list_get(valores, 1));
			x = *((u_int32_t*) list_get(valores, 2));
			y = *((u_int32_t*) list_get(valores, 3));
			// Mover el tripulante
			mover(pid, tid, x, y);
			log_info(logger, "Muevo tripulante %d (P%d) a (%d,%d)", tid, pid, x, y);
			break;
		case PROXIMA_TAREA:
			valores = recibir_paquete(cliente);
			// Obtener los valores
			tid = *((u_int32_t*) list_get(valores, 0));
			pid = *((u_int32_t*) list_get(valores, 1));			
			// Enviar la tarea
			enviar_proxima_tarea(tid, pid, cliente);
			break;
		default:
			log_info(logger, "Operacion no reconocida!");
			break;
	}

	if (valores != NULL) list_destroy_and_destroy_elements(valores, free);
}

void signal_handler(int sig) {
	switch (sig) {
		case SIGUSR1:
			if (esquema == SEGMENTACION) seg_dump();
			if (esquema == PAGINACION) pag_dump(false); // "true" para imprimir la virtual
			break;
		case SIGUSR2:
			if (esquema != SEGMENTACION) break;
			sem_wait(&mutex_memoria);
			compactacion();
			sem_post(&mutex_memoria);
			break;
		default: break;
	}
}

void iniciar_memoria(t_config* config) {
	int tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	memoria_principal = malloc(tamanio_memoria);
	sem_init(&mutex_memoria, 0, 1);

	esquema = strcmp(config_get_string_value(config, "ESQUEMA_MEMORIA"), "PAGINACION") == 0 ? PAGINACION : SEGMENTACION;
	if (esquema == PAGINACION) {
		int algoritmo_reemplazo = strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO"), "LRU") == 0 ? LRU : CLOCK;
		char* path_swap = config_get_string_value(config, "PATH_SWAP");
		int tamanio_pag = config_get_int_value(config, "TAMANIO_PAGINA");
		int tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP");
		pag_iniciar_variables(memoria_principal, tamanio_memoria, algoritmo_reemplazo, path_swap, tamanio_pag, tamanio_swap);
	} else {
		int criterio_seleccion = strcmp(config_get_string_value(config, "CRITERIO_SELECCION"), "FF") == 0 ? FF : BF;
		seg_iniciar_variables(memoria_principal, tamanio_memoria, criterio_seleccion);
	}
	signal(SIGUSR1, &signal_handler);
	signal(SIGUSR2, &signal_handler);
}