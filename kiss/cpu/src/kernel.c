#include "kernel.h"

bool desalojar;
pthread_mutex_t mutex_desalojar;
t_list *instrucciones;
int conexion_dispatch;

void interrupcion(int conexion, t_log *logger) {
    pthread_mutex_lock(&mutex_desalojar);
    desalojar = true;
    pthread_mutex_unlock(&mutex_desalojar);
    log_info(logger, "Se recibio una interrupcion!!");
}

void ejecutar_proceso(int conexion, t_log *logger) {
    conexion_dispatch = conexion;
    t_list *valores = recibir_paquete(conexion);
    pcb = deserializar_pcb(list_get(valores, 0));
    list_destroy(valores);
    pthread_mutex_lock(&mutex_desalojar);
    desalojar = false;
    pthread_mutex_unlock(&mutex_desalojar);
    instrucciones = deserializar_instrucciones(pcb->instrucciones);
    vaciar_tlb();
    fetch();
}

void recibir_handshake(int conexion, t_log *logger) {
    enviar_numero(conexion, HANDSHAKE_CPU);
}

t_list *handlers_interrupt() {
    t_list *handlers = list_create();
    agregar_handler(handlers, HANDSHAKE_KERNEL, &recibir_handshake);
    agregar_handler(handlers, INTERRUPCION, &interrupcion);
    return handlers;
}

t_list *handlers_dispatch() {
    t_list *handlers = list_create();
    agregar_handler(handlers, HANDSHAKE_KERNEL, &recibir_handshake);
    agregar_handler(handlers, EJECUTAR_PROCESO, &ejecutar_proceso);
    return handlers;
}
