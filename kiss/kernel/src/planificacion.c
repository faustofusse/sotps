#include "planificacion.h"

#define MS_BETWEEN_IO_CHECK 50

uint32_t next_id;
sem_t next_id_mutex,
    multiprogramacion,
    blocked,
    ready,
    entrando_a_ready,
    ejecutando,
    planificacion
;
pthread_t hilo_corto_plazo, hilo_largo_plazo, hilo_cpu, hilo_dispositivo_io;

// -------- UTILS -------------------------------------------

bool hay_desalojo() {
    return kernel_config->algoritmo_planificacion == SRT;
}

// -------- LARGO PLAZO -------------------------------------

t_proceso* crear_proceso(uint32_t id, uint32_t tamanio, char* instrucciones, int conexion) {
    t_proceso* p = malloc(sizeof(t_proceso));
    p->conexion = conexion;
    p->bloqueo_restante = 0;
    p->tiempo_bloqueado = 0;
    p->suspendido = false;
    p->pcb = crear_pcb(id, tamanio, instrucciones, kernel_config->estimacion_inicial);
    return p;
}

void iniciar_proceso(uint32_t tamanio, char* instrucciones, int conexion) {
    sem_wait(&next_id_mutex);
    uint32_t pid = next_id++;
    sem_post(&next_id_mutex);
    t_proceso* proceso = crear_proceso(pid, tamanio, instrucciones, conexion);
    /* iniciar_en_memoria(proceso); // esto se tiene que hacer una vez que este en ready sino no hay lugar en memoria */
    push(NEW, proceso);
    sem_post(&entrando_a_ready);
}

void destruir_proceso(t_proceso* p) {
    destruir_pcb(p->pcb);
    free(p);
}

void finalizar_proceso(t_proceso* p) {
    finalizar_en_memoria(p);
    sacar_proceso(EXIT, p);
    enviar_numero(p->conexion, FINALIZACION_CONSOLA);
    destruir_proceso(p);
}

// -------- CORTO PLAZO -------------------------------------

void desalojar_proceso() {
    enviar_numero(conexion_interrupt, INTERRUPCION);
}

bool comparar_estimacion(void *a, void *b) {
    return ((t_proceso *)a)->pcb->estimacion_rafaga < ((t_proceso *)b)->pcb->estimacion_rafaga;
}

void ordenar_por_estimacion(t_list* lista, sem_t mutex) {
    sem_wait(&mutex);
    list_sort(lista, &comparar_estimacion);
    sem_post(&mutex);
}

uint64_t get_timestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void actualizar_estimacion(t_proceso* p, bool recalcular) {
    float alfa = kernel_config->alfa;
    uint32_t ultima_rafaga = p->pcb->estimacion_rafaga;
    uint64_t tiempo_ejecucion = get_timestamp() - p->timestamp;
    uint64_t estimacion = recalcular ?
        alfa * tiempo_ejecucion + (1 - alfa) * ultima_rafaga
        : ultima_rafaga - tiempo_ejecucion;
    p->pcb->estimacion_rafaga = (uint32_t) estimacion;
    log_info(logger, "Estimacion P%d: %d", p->pcb->id, p->pcb->estimacion_rafaga);
}

void reubicar_proceso(t_proceso* p, int viejo, int nuevo) {
    sacar_proceso(viejo, p);
    if (nuevo == READY) {
        insert_sorted(nuevo, p);
        sem_post(&ready);
    } else push(nuevo, p);
    if (nuevo == EXIT) finalizar_proceso(p);
    if (nuevo == BLOCKED) sem_post(&blocked);
    if ((viejo == READY || viejo == BLOCKED || viejo == EXEC) && nuevo == EXIT)
        sem_post(&multiprogramacion);
}

void ejecutar_proceso(t_proceso* p) {
    // Me guardo el timestamp
    p->timestamp = get_timestamp();
    // Le paso el PCB a la CPU
    enviar_proceso(p->pcb);
    // Espero que la CPU me devuelva el PCB actualizado
    int nuevo_estado = recibir_pcb(p);
    // Actualizar estimacion (para SRT)
    if (kernel_config->algoritmo_planificacion == SRT && nuevo_estado != EXIT)
        actualizar_estimacion(p, nuevo_estado == BLOCKED);
    // Pongo el proceso en la cola correspondiente
    reubicar_proceso(p, EXEC, nuevo_estado);
}

// -------- MEDIANO PLAZO -----------------------------------

void suspender_procesos() {
    for (int i = 0; i < list_size(colas[BLOCKED]); i++) {
        t_proceso* p = list_get(colas[BLOCKED], i);
        p->tiempo_bloqueado += MS_BETWEEN_IO_CHECK;
        if (p->tiempo_bloqueado >= kernel_config->tiempo_maximo_bloqueo && !p->suspendido) {
            suspender_en_memoria(p);
            p->suspendido = true;
            log_info(logger, "Proceso %d -> SUSPENDED_BLOCK", p->pcb->id);
            sem_post(&multiprogramacion);
        }
    }
}

void realizar_io() {
    t_proceso* p = peek(BLOCKED);
    usleep(MS_BETWEEN_IO_CHECK * 1000);
    p->bloqueo_restante -= MS_BETWEEN_IO_CHECK;
    if (p->bloqueo_restante > 0) return;
    pop(BLOCKED);
    sem_wait(&blocked);
    if (p->suspendido) {
        push(SUSPENDED_READY, p);
        sem_post(&entrando_a_ready);
    } else {
        push(READY, p);
        sem_post(&ready);
    }
}

// -------- HILOS -------------------------------------------

// Transiciones: NEW -> READY, SUSPENDED_READY -> READY
void planificador_largo_plazo() {
    while (1) {
        sem_wait(&multiprogramacion);
        sem_wait(&entrando_a_ready);
        t_proceso* p = pop(SUSPENDED_READY);
        if (p == NULL) {
            p = pop(NEW);
            iniciar_en_memoria(p);
        }
        p->suspendido = false;
        push(READY, p);
        sem_post(&ready);
    }
}

// Transiciones: READY -> EXEC
void planificador_corto_plazo() {
    while (1) {
        sem_wait(&ready);
        if (hay_desalojo() && peek(EXEC) != NULL)
            desalojar_proceso();
        sem_wait(&planificacion);
        push(EXEC, pop(READY));
        sem_post(&ejecutando);
    }
}

// Transiciones: EXEC -> READY, EXEC -> BLOCKED, EXEC -> EXIT
void cpu() {
    while (1) {
        sem_wait(&ejecutando);
        ejecutar_proceso(peek(EXEC));
        sem_post(&planificacion);
    }
}

// Transiciones: BLOCKED -> READY, BLOCKED -> SUSP_BLOCKED, SUSP_BLOCKED -> SUSP_READY
void dispositivo_io() {
    while (1) {
        sem_wait(&blocked);
        suspender_procesos();
        sem_post(&blocked);
        realizar_io();
    }
}

// -------- GENERAL -----------------------------------------

void iniciar_planificacion() {
    // Colas y mutexs de colas
    for (int i = 0; i < CANTIDAD_ESTADOS; i++) {
        colas[i] = list_create();
        sem_init(&semaforos[i], 0, 1);
    }
    // Contador de PID
    next_id = 1;
    // Semaforos
    sem_init(&next_id_mutex, 0, 1);
    sem_init(&multiprogramacion, 0, kernel_config->grado_multiprogramacion);
    sem_init(&ready, 0, 0);
    sem_init(&blocked, 0, 0);
    sem_init(&entrando_a_ready, 0, 0);
    sem_init(&ejecutando, 0, 0);
    sem_init(&planificacion, 0, 1);
    // Hilos de planificacion
    pthread_create(&hilo_largo_plazo, NULL, (void*) planificador_largo_plazo, NULL);
    pthread_detach(hilo_largo_plazo);
    pthread_create(&hilo_corto_plazo, NULL, (void*) planificador_corto_plazo, NULL);
    pthread_detach(hilo_corto_plazo);
    pthread_create(&hilo_cpu, NULL, (void*) cpu, NULL);
    pthread_detach(hilo_cpu);
    pthread_create(&hilo_dispositivo_io, NULL, (void*) dispositivo_io, NULL);
    pthread_detach(hilo_dispositivo_io);
}

void finalizar_planificacion() {
    for (int i = 0; i < CANTIDAD_ESTADOS; i++) {
        sem_wait(&semaforos[i]);
        for (int j = 0; j < list_size(colas[i]); j++)
            enviar_numero(((t_proceso*) list_get(colas[i], j))->conexion, FINALIZACION_CONSOLA);
        sem_post(&semaforos[i]);
    }
    pthread_kill(hilo_corto_plazo, SIGABRT);
    pthread_kill(hilo_largo_plazo, SIGABRT);
    pthread_kill(hilo_cpu, SIGABRT);
    pthread_kill(hilo_dispositivo_io, SIGABRT);
}
