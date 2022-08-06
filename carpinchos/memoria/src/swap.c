#include "swap.h"

int swap_iniciar_carpincho(int pid) {
     t_paquete* paquete = crear_paquete(HANDSHAKE_CARPINCHO);
     agregar_a_paquete(paquete, &pid, sizeof(int));
     enviar_paquete(paquete, conexion_swap);
     eliminar_paquete(paquete);
     return recibir_numero(conexion_swap);
}

void* swap_leer_pagina(int pid, int pagina) {
     t_paquete* paquete = crear_paquete(PAGE_READ);
     agregar_a_paquete(paquete, &pid, sizeof(int));
     agregar_a_paquete(paquete, &pagina, sizeof(int));
     enviar_paquete(paquete, conexion_swap);
     eliminar_paquete(paquete);
     int mensaje = recibir_operacion(conexion_swap);
     if (mensaje == MATE_READ_FAULT) return NULL;
     t_list* valores = recibir_paquete(conexion_swap);
     void* contenido = list_get(valores, 0);
     list_destroy(valores);
     return contenido;
}

int swap_escribir_pagina(int pid, int pagina, void* contenido) {
     t_paquete* paquete = crear_paquete(PAGE_WRITE);
     agregar_a_paquete(paquete, &pid, sizeof(int));
     agregar_a_paquete(paquete, &pagina, sizeof(int));
     agregar_a_paquete(paquete, contenido, tamanio_pagina);
     enviar_paquete(paquete, conexion_swap);
     eliminar_paquete(paquete);
     return recibir_numero(conexion_swap);
}

int swap_eliminar_pagina(int pid, int pagina) {
     t_paquete* paquete = crear_paquete(PAGE_REMOVE);
     agregar_a_paquete(paquete, &pid, sizeof(int));
     agregar_a_paquete(paquete, &pagina, sizeof(int));
     enviar_paquete(paquete, conexion_swap);
     eliminar_paquete(paquete);
     return recibir_numero(conexion_swap);
}

void swap_eliminar_proceso(int pid) {
     t_paquete* paquete = crear_paquete(CLOSE_CARPINCHO);
     agregar_a_paquete(paquete, &pid, sizeof(int));
     enviar_paquete(paquete, conexion_swap);
     eliminar_paquete(paquete);
}