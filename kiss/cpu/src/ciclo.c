#include "ciclo.h"

void fetch() {
    log_info(logger, "---------------------------------------");
    /* log_info(logger, "(P%d) FETCH instruccion %d", pcb->id, pcb->program_counter); */
    t_instruccion* proxima = list_get(instrucciones, pcb->program_counter);
    pcb->program_counter++;
    decode(proxima);
}

bool necesita_operandos(t_instruccion* instruccion) {
    return strcmp(instruccion->identificador, "COPY") == 0;
}

void decode(t_instruccion* instruccion) {
    /* log_info(logger, "(P%d) DECODE instruccion '%s'", pcb->id, instruccion->identificador); */
    t_list* operands = necesita_operandos(instruccion) ? fetch_operands(instruccion) : NULL;
    t_executable ejecutable = get_executable(instruccion->identificador);
    execute(instruccion, operands, ejecutable);
}

t_list* fetch_operands(t_instruccion* instruccion) {
    log_info(logger, "(P%d) FETCH OPERANDS instruccion '%s'", pcb->id, instruccion->identificador);
    t_list* operands = NULL;
    if (strcmp(instruccion->identificador, "COPY") == 0) {
        operands = list_create();
        uint32_t direccion_logica_origen = *((uint32_t*) list_get(instruccion->parametros, 1));
        uint32_t* valor = malloc(sizeof(uint32_t));
        *valor = memoria_read(direccion_logica_origen);
        list_add(operands, valor);
    }
    return operands;
}

void execute(t_instruccion* instruccion, t_list* operands, t_executable ejecutable) {
    log_info(logger, "(P%d) EXEC instruccion %d '%s' (%d ops)", pcb->id, pcb->program_counter, instruccion->identificador, operands == NULL ? 0 : list_size(operands));
    bool seguir = ejecutable(instruccion->parametros, operands);
    if (operands != NULL) list_destroy_and_destroy_elements(operands, free);
    if (seguir) check_interrupt();
}

void check_interrupt() {
    pthread_mutex_lock(&mutex_desalojar);
    bool seguir_ejecutando = !desalojar;
    pthread_mutex_unlock(&mutex_desalojar);
    if (!seguir_ejecutando) {
        log_info(logger, "Desalojo el proceso %d", pcb->id);
        devolver_proceso(READY);
    } else fetch();
}

