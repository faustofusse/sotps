#include "tlb.h"

void pruebas_tlb() { dump_tlb(); }

void iniciar_tlb() { tlb = list_create(); }

void limpiar_tlb() { list_clean(tlb); }

void destruir_tlb() { list_destroy(tlb); }

void sacar_de_tlb(t_pagina* pagina) {
     for (int i = 0; i < list_size(tlb); i++)
          if (list_get(tlb, i) == pagina) {
               list_remove(tlb, i);
               return;
          }
}

void sacar_carpincho_de_tlb(int pid) {
     for (int i = 0; i < list_size(tlb); i++) {
          t_pagina* pagina = list_get(tlb, i);
          if (pagina->pid == pid) list_remove(tlb, i);
     }
}

void agregar_a_tlb(t_pagina* pagina) {
     if (list_size(tlb) >= entradas_tlb) list_remove(tlb, 0);
     list_add(tlb, pagina);
}

uint32_t buscar_en_tlb(t_carpincho* carpincho, uint32_t numero) {
     t_pagina* pagina;
     for (int i = 0; i < list_size(tlb); i++) {
          pagina = list_get(tlb, i);
          if (pagina->pid != carpincho->pid || pagina->numero != numero) continue;
          if (algoritmo_tlb == LRU) {
               list_remove(tlb, i);
               list_add(tlb, pagina);
          }
          printf("TLB HIT: PID: %d, Pagina: %d, Marco: %d\n", carpincho->pid, numero, pagina->frame);
          hits++;
          carpincho->hits++;
          sleep(retardo_hit);
          return pagina->frame;
     }
     printf("TLB MISS: PID: %d, Pagina: %d\n", carpincho->pid, numero);
     misses++;
     carpincho->misses++;
     sleep(retardo_miss);
     return -1;
}

void dump_tlb() {
     char* tiempo = temporal_get_string_time("Dump_%y%m%d%H%M%S.tlb");
     char* nombre = string_from_format("%s/%s", path_dump, tiempo);
     FILE* archivo = fopen(nombre, "w");
     free(nombre);
     free(tiempo);
     if (archivo == NULL) { printf("No se pudo abrir el archivo del dump\n"); return; }
     fprintf(archivo, "------------------------------------------------------------------------------------------\n");
     tiempo = temporal_get_string_time("Dump: %d/%m/%y %H:%M:%S\n");
     fprintf(archivo, "%s", tiempo);
     free(tiempo);
     int cantidad = list_size(tlb);
     for (int i = 0; i < entradas_tlb; i++) {
          if (i < cantidad) {
               t_pagina *pagina = list_get(tlb, i);
               fprintf(archivo, "Entrada: %d\tEstado: Ocupado\t\tCarpincho: %d\tPagina: %d\tMarco: %d\n", i, pagina->pid, pagina->numero, pagina->frame);
          } else fprintf(archivo, "Entrada: %d\tEstado: Libre\t\tCarpincho: -\tPagina: -\tMarco: -\n", i);
     }
     fprintf(archivo, "------------------------------------------------------------------------------------------\n");
}