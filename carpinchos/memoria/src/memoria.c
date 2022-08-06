#include "memoria.h"

int ultimo_id = 1;

void get_config() {
     tamanio_memoria = config_get_int_value(config, "TAMANIO");
     tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
     tipo_asignacion = strcmp(config_get_string_value(config, "TIPO_ASIGNACION"), "FIJA") == 0 ? FIJA : DINAMICA;
     marcos_por_carpincho = config_get_int_value(config, "MARCOS_POR_CARPINCHO");
     algoritmo_mmu = strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO_MMU"), "CLOCK-M") == 0 ? CLOCK : LRU;
     algoritmo_tlb = strcmp(config_get_string_value(config, "ALGORITMO_REEMPLAZO_TLB"), "FIFO") == 0 ? FIFO : LRU;
     entradas_tlb = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
     retardo_hit = config_get_int_value(config, "RETARDO_ACIERTO_TLB");
     retardo_miss = config_get_int_value(config, "RETARDO_FALLO_TLB");
     path_dump = config_get_string_value(config, "PATH_DUMP_TLB");
}

int main(void) {
     logger = crear_logger();
     config = crear_config();

     get_config();

     iniciar_memoria();

     iniciar_tlb();

     conexion_swap = conectar_swamp(config);

     int servidor = crear_servidor(config_get_int_value(config, "PUERTO"));
     escuchar(servidor, logger, *recibir);

     return EXIT_SUCCESS;
}

void recibir(int cliente, int operacion) {
     t_list* valores = NULL;
     void* contenido;
     int direccion, tamanio, pid;
     t_paquete* paquete;
     switch (operacion) {
          case MEM_MALLOC:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               tamanio = *((int*) list_get(valores, 1));
               mate_pointer puntero = memalloc(pid, tamanio);
               printf("Malloc de size %d -> Puntero: %d\n", tamanio, puntero);
               enviar_numero(cliente, puntero);
               break;
          case MEM_FREE:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               direccion = *((int*) list_get(valores, 1));
               printf("Free de la direccion %d\n", direccion);
               memfree(pid, direccion);
               break;
          case MEM_READ:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               direccion = *((int*) list_get(valores, 1));
               tamanio = *((int*) list_get(valores, 2));
               printf("Read de la direccion '%d' de tamanio '%d'\n", direccion, tamanio);
               contenido = memread(pid, direccion, tamanio);
               paquete = crear_paquete(MEM_READ);
               agregar_a_paquete(paquete, contenido, sizeof(int));
               enviar_paquete(paquete, cliente);
               eliminar_paquete(paquete);
               free(contenido);
               break;
          case MEM_WRITE:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               direccion = *((int*) list_get(valores, 1));
               tamanio = *((int*) list_get(valores, 2));
               contenido = list_get(valores, 3);
               printf("Write en la direccion '%d', de tamanio '%d'\n", direccion, tamanio);
               memwrite(pid, contenido, direccion, tamanio);
               break;
          case CLOSE_CARPINCHO:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               log_info(logger, "Eliminando el carpincho %d!", pid);
               t_paquete* paquete = crear_paquete(CLOSE_CARPINCHO);
               agregar_a_paquete(paquete, &pid, sizeof(int));
               enviar_paquete(paquete, conexion_swap);
               eliminar_paquete(paquete);
               eliminar_carpincho(pid);
               break;
          case HANDSHAKE_CARPINCHO:
               log_info(logger, "Se conecto un carpincho!");
               enviar_numero(cliente, HANDSHAKE_MEMORIA);
               pid = ultimo_id++;
               enviar_numero(cliente, pid);
               iniciar_carpincho(pid);
               break;
          case HANDSHAKE_KERNEL:
               log_info(logger, "Se conecto el kernel!");
               enviar_numero(cliente, HANDSHAKE_MEMORIA);
               break;
          default:
               log_info(logger, "Operacion no reconocida!");
               break;
     }
     if (valores != NULL) list_destroy_and_destroy_elements(valores, free);
}

t_config* crear_config() {
     return config_create("./config.cfg");
}

t_log* crear_logger() {
     return log_create("./memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
}

int conectar_swamp(t_config* config) {
     // Creo la conexion
     int memoria = crear_conexion(
          config_get_string_value(config, "IP_SWAMP"),
          config_get_int_value(config, "PUERTO_SWAMP")
     );
     // Hago el handshake
     enviar_numero(memoria, HANDSHAKE_MEMORIA);
     // Recibo la respuesta de la memoria
     int respuesta = recibir_operacion(memoria);
     if (respuesta == HANDSHAKE_SWAMP) {
          enviar_numero(memoria, tipo_asignacion);
          log_info(logger, "Se conecto la swap!");
     } else {
          log_info(logger, "No se pudo conectar la swap!");
          exit(EXIT_FAILURE);
     }
     // Devuelvo el cliente
     return memoria;
}