#include "execute.h"

void _devolver_proceso(uint32_t estado, uint32_t info) {
    void* serializado = serializar_pcb(pcb);
    t_paquete* paquete = crear_paquete(DEVOLVER_PROCESO);
    agregar_a_paquete(paquete, &estado, sizeof(uint32_t));
    agregar_a_paquete(paquete, serializado, tamanio_pcb(pcb));
    if (info != 0) agregar_a_paquete(paquete, &info, sizeof(uint32_t));
    enviar_paquete(paquete, conexion_dispatch);
    eliminar_paquete(paquete);
    free(serializado);
    destruir_instrucciones(instrucciones);
    destruir_pcb(pcb);
}

void devolver_proceso(uint32_t estado) {
    _devolver_proceso(estado, 0);
}

bool execute_no_op(t_list* parametros, t_list* operands) {
    usleep(cpu_config->retardo_noop * 1000);
    return true;
}

bool execute_io(t_list* parametros, t_list* operands) {
    uint32_t tiempo = *((uint32_t*) list_get(parametros, 0));
    _devolver_proceso(BLOCKED, tiempo);
    return false;
}

bool execute_read(t_list* parametros, t_list* operands) {
    uint32_t direccion_logica = *((uint32_t*) list_get(parametros, 0));
    memoria_read(direccion_logica);
    return true;
}

bool execute_write(t_list* parametros, t_list* operands) {
    uint32_t direccion_logica = *((uint32_t*) list_get(parametros, 0));
    uint32_t valor = *((uint32_t*) list_get(parametros, 1));
    memoria_write(direccion_logica, valor);
    return true;
}

bool execute_copy(t_list* parametros, t_list* operands) {
    uint32_t direccion_logica_destino = *((uint32_t*) list_get(parametros, 0));
    uint32_t valor = *((uint32_t*) list_get(operands, 0));
    memoria_write(direccion_logica_destino, valor);
    return true;
}

bool execute_exit(t_list* parametros, t_list* operands) {
    devolver_proceso(EXIT);
    return false;
}

t_executable get_executable(char* identificador) {
    if (strcmp(identificador, "NO_OP") == 0) return execute_no_op;
    else if (strcmp(identificador, "I/O") == 0) return execute_io;
    else if (strcmp(identificador, "READ") == 0) return execute_read;
    else if (strcmp(identificador, "WRITE") == 0) return execute_write;
    else if (strcmp(identificador, "COPY") == 0) return execute_copy;
    else if (strcmp(identificador, "EXIT") == 0) return execute_exit;
    log_error(logger, "Instruccion invalida: %s", identificador);
    return NULL;
}

