#include "cpu.h"

int conexion_dispatch, conexion_interrupt;

int conexion_y_handshake(int puerto, char* name) {
    int conexion = crear_conexion(kernel_config->ip_cpu, puerto);
    enviar_numero(conexion, HANDSHAKE_KERNEL);
    int confirmacion = recibir_numero(conexion);
    if (confirmacion != HANDSHAKE_CPU) {
        log_error(logger, "Error al conectar con %s (CPU)! Reintentando...", name);
        sleep(2);
        conexion_y_handshake(puerto, name);
    } else log_info(logger, "Me conecte con %s (CPU).", name);
    return conexion;
}

void conectar_cpu() {
    // Conexion con dispatch
    conexion_dispatch = conexion_y_handshake(kernel_config->puerto_cpu_dispatch, "dispatch");
    // Conexion con interrupt
    conexion_interrupt = conexion_y_handshake(kernel_config->puerto_cpu_interrupt, "interrupt");
}

void enviar_interrupcion() {
    enviar_numero(conexion_interrupt, INTERRUPCION);
}

void enviar_proceso(t_pcb* pcb) {
    t_paquete* paquete = crear_paquete(EJECUTAR_PROCESO);
    void* serializado = serializar_pcb(pcb);
    agregar_a_paquete(paquete, serializado, tamanio_pcb(pcb));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
    free(serializado);
}

int recibir_pcb(t_proceso* p) {
    int operacion = recibir_numero(conexion_dispatch);
    if (operacion != DEVOLVER_PROCESO) {
        log_error(logger, "Se esperaba la devolucion del proceso pero se recibio otra cosa.");
        return -1;
    }
    t_list* valores = recibir_paquete(conexion_dispatch);
    uint32_t nuevo_estado = *((uint32_t*) list_get(valores, 0));
    t_pcb* pcb_actualizado = deserializar_pcb(list_get(valores, 1));
    destruir_pcb(p->pcb);
    p->pcb = pcb_actualizado;
    free(list_get(valores, 0));
    if (nuevo_estado == BLOCKED) {
        uint32_t tiempo = *((uint32_t*) list_get(valores, 2));
        p->bloqueo_restante = tiempo;
        p->tiempo_bloqueado = 0;
        free(list_get(valores, 2));
    }
    list_destroy(valores);
    return nuevo_estado;
}
