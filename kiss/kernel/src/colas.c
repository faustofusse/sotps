#include "colas.h"

t_list *colas[CANTIDAD_ESTADOS];
sem_t semaforos[CANTIDAD_ESTADOS];

char* estado_string(int estado) {
    switch (estado) {
        case NEW: return "NEW";
        case READY: return "READY";
        case EXEC: return "EXEC";
        case BLOCKED: return "BLOCK";
        case SUSPENDED_BLOCKED: return "SUSPENDED_BLOCK";
        case SUSPENDED_READY: return "SUSPENDED_READY";
        case EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}

void sacar_proceso(int cola, t_proceso* p) {
    bool condicion(void* proceso) {
        return proceso == p;
    }
    sem_wait(&semaforos[cola]);
    list_remove_by_condition(colas[cola], &condicion);
    sem_post(&semaforos[cola]);
}

void push(int cola, t_proceso *p) {
    sem_wait(&semaforos[cola]);
    list_add(colas[cola], p);
    sem_post(&semaforos[cola]);
    log_info(logger, "Proceso %d -> %s", p->pcb->id, estado_string(cola));
}

t_proceso* pop(int cola) {
    sem_wait(&semaforos[cola]);
    t_proceso *p = list_size(colas[cola]) == 0 ? NULL : list_remove(colas[cola], 0);
    sem_post(&semaforos[cola]);
    return p;
}

t_proceso* peek(int cola) {
    sem_wait(&semaforos[cola]);
    t_proceso *p = list_size(colas[cola]) == 0 ? NULL : list_get(colas[cola], 0);
    sem_post(&semaforos[cola]);
    return p;
}

void insert_sorted(int cola, t_proceso* p) {
    sem_wait(&semaforos[cola]);
    int i = 0, size = list_size(colas[cola]);
    for (i = 0; i < size; i++) {
        if (((t_proceso*) list_get(colas[cola], i))->pcb->estimacion_rafaga > p->pcb->estimacion_rafaga) {
            list_add_in_index(colas[cola], i, p);
            break;
        }
    }
    if (i == size) list_add(colas[cola], p);
    sem_post(&semaforos[cola]);
    log_info(logger, "Proceso %d -> %s", p->pcb->id, estado_string(cola));
}

