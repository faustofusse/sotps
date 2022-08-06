#include "memoria.h"

int conexion_memoria;

void conectar_memoria() {
    conexion_memoria = crear_conexion(kernel_config->ip_memoria, kernel_config->puerto_memoria);
    enviar_numero(conexion_memoria, HANDSHAKE_KERNEL);
    if (recibir_numero(conexion_memoria) != HANDSHAKE_MEMORIA) {
        log_error(logger, "Error al conectar con memoria, reintentando...");
        sleep(2);
        conectar_memoria();
    }
}

void iniciar_en_memoria(t_proceso* p) {
    // Enviar mensaje a memoria
    t_paquete* paquete = crear_paquete(INICIAR_PROCESO);
    agregar_a_paquete(paquete, &p->pcb->id, sizeof(uint32_t));
    agregar_a_paquete(paquete, &p->pcb->tamanio, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    // Recibir puntero a tabla de paginas
    uint32_t tabla_paginas = recibir_numero(conexion_memoria);
    // Actualizar pcb
    p->pcb->tabla_paginas = tabla_paginas;
}

void finalizar_en_memoria(t_proceso* p) {
    // Enviar mensaje a memoria
    t_paquete* paquete = crear_paquete(FINALIZAR_PROCESO);
    agregar_a_paquete(paquete, &p->pcb->tabla_paginas, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    // Recibir confirmacion
    uint32_t confirmacion = recibir_numero(conexion_memoria);
    if (confirmacion != FINALIZAR_PROCESO) {
        log_error(logger, "Erro al finalizar el proceso en memoria");
    }
}

void suspender_en_memoria(t_proceso* p) {
    // Enviar mensaje a memoria
    t_paquete* paquete = crear_paquete(SUSPENDER_PROCESO);
    agregar_a_paquete(paquete, &p->pcb->tabla_paginas, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
    // Recibir confirmacion
    uint32_t confirmacion = recibir_numero(conexion_memoria);
    if (confirmacion != SUSPENDER_PROCESO) {
        log_error(logger, "Erro al suspender el proceso en memoria");
    }
}

