#include "paginacion.h"

// t_carpincho* buscar_carpincho(uint32_t id) {
//      for (int i = 0; i < list_size(carpinchos); i++) {
//           t_carpincho* c = list_get(carpinchos, i);
//           if (c->pid == id) return c;
//      }
//      return NULL;
// }

// void* comprimir_metadata(HeapMetadata metadata) {
//      void* comprimido = malloc(METADATA_SIZE);
//      memcpy(comprimido, &(metadata.prevAlloc), sizeof(uint32_t));
//      memcpy(comprimido + sizeof(uint32_t), &(metadata.nextAlloc), sizeof(uint32_t));
//      memcpy(comprimido + sizeof(uint32_t) * 2, &(metadata.isFree), sizeof(uint8_t));
//      return comprimido;
// }

// HeapMetadata descomprimir_metadata(void* direccion) {
//      HeapMetadata metadata;
//      memcpy(&(metadata.prevAlloc), direccion, sizeof(uint32_t));
//      memcpy(&(metadata.nextAlloc), direccion + sizeof(uint32_t), sizeof(uint32_t));
//      memcpy(&(metadata.isFree), direccion + sizeof(uint32_t) * 2, sizeof(uint8_t));
//      return metadata;
// }

// HeapMetadata nuevo_metadata(uint32_t prevAlloc, uint32_t nextAlloc, uint8_t isFree) {
//      HeapMetadata metadata;
//      metadata.prevAlloc = prevAlloc;
//      metadata.nextAlloc = nextAlloc;
//      metadata.isFree = isFree;
//      return metadata;
// }

// int reservar_frames() {
//      int frame = -1;
//      for (int i = 0; i < tamanio_memoria / tamanio_pagina / marcos_por_carpincho; i++) 
//           if (!bitarray_test_bit(bitmap_procesos, i)) {
//                frame = i;
//                bitarray_set_bit(bitmap_procesos, i);
//                break;
//           }
//      return frame == -1 ? -1 : frame * marcos_por_carpincho;
// }

// int memfree(uint32_t pid, mate_pointer addr) {
//      return 0;
// }

// void* memread(uint32_t pid, mate_pointer addr, int size) {
//      void* contenido = malloc(sizeof(int));
//      int a = 3;
//      memcpy(contenido, &a, sizeof(int));
//      return contenido;
// }

// int memwrite(uint32_t pid, void *contenido, mate_pointer addr, int size) {
//      return 0;
// }

// mate_pointer memalloc(uint32_t pid, int size) {
//      // t_carpincho* carpincho = buscar_carpincho(pid);
//      // int frame = buscar_pagina(carpincho, 0);
//      mate_pointer puntero = 3;
//      dump_memoria();
//      return puntero;
// }


// void* pagina_vacia() {
//      void* pagina = malloc(tamanio_pagina);
//      memset(pagina, 0, tamanio_pagina);
//      void* comprimido = comprimir_metadata(nuevo_metadata(0, 0, true));
//      memcpy(pagina, comprimido, METADATA_SIZE);
//      free(comprimido);
//      return pagina;
// }


// void iniciar_carpincho(uint32_t pid) {
//      // Creo el struct
//      t_carpincho* c = malloc(sizeof(t_carpincho));
//      c->pid = pid;
//      c->hits = 0;
//      c->misses = 0;
//      c->paginas = list_create();
//      c->residentes = tipo_asignacion == FIJA ? list_create() : paginas;
//      c->primer_marco = tipo_asignacion == FIJA ? reservar_frames() : 0;
//      // Si no hay lugar, tiro error
//      if (c->primer_marco == -1) {
//           list_destroy(c->paginas);
//           free(c);
//           log_error(logger, "No hay lugar para el proceso %d", pid);
//           return;
//      }
//      // Le aviso a la swap
//      if (swap_iniciar_carpincho(pid) == -1) {
//           log_error(logger, "Error al iniciar el carpincho (en la swap)");
//           return;
//      }
//      // Creo la primera pagina (vacia) y la agrego a memoria
//      t_pagina* p = malloc(sizeof(t_pagina));
//      p->pid = pid;
//      p->numero = 0;
//      list_add(c->paginas, p);
//      list_add(carpinchos, c);
//      // La escribo en swap primero
//      void* contenido = pagina_vacia();
//      if (swap_escribir_pagina(p->pid, p->numero, contenido) == -1) {
//           free(contenido);
//           log_error(logger, "Error al escribir la primer pagina del carpincho (en la swap)");
//           return;
//      }
//      // La inserto en la memoria real
//      insertar_pagina(c, p, contenido);
//      // Pruebasss
//      dump_memoria();
// }

// void eliminar_carpincho(uint32_t pid) {
//      t_carpincho* c;
//      // Lo saco de la lista de procesos
//      for (int i = 0; i < list_size(carpinchos); i++) {
//           c = list_get(carpinchos, i);
//           if (c->pid != pid && i == list_size(carpinchos) - 1) return;
//           else if (c->pid != pid) continue;
//           list_remove(carpinchos, i);
//           break;
//      }
//      // Elimino las paginas
//      int cantidad = list_size(c->paginas);
//      for (int i = 0; i < cantidad; i++) eliminar_pagina(c, 0);
//      list_destroy(c->paginas);
//      // Libero la reserva de paginas (si la asignacion es fija)
//      if (tipo_asignacion == FIJA) {
//           bitarray_clean_bit(bitmap_procesos, c->primer_marco / marcos_por_carpincho);
//           list_destroy(c->residentes);
//      }
//      // Libero memoria
//      free(c);
// }

// void iniciar_memoria() {
//      // Memoria principal
//      memoria_principal = malloc(tamanio_memoria);
//      // Bitarray marcos totales
//      int tamanio_bitarray = tamanio_memoria / tamanio_pagina;
//      void* bits = malloc(tamanio_bitarray / 8 + 1);
//      memset(bits, 0, tamanio_bitarray / 8 + 1);
//      bitmap_memoria = bitarray_create(bits, tamanio_bitarray);
//      // Bitarray procesos
//      tamanio_bitarray = tamanio_memoria / tamanio_pagina / marcos_por_carpincho;
//      bits = malloc(tamanio_bitarray / 8 + 1);
//      memset(bits, 0, tamanio_bitarray / 8 + 1);
//      bitmap_procesos = bitarray_create(bits, tamanio_bitarray);
//      // Listas
//      carpinchos = list_create();
//      paginas = list_create();
// }

// bool comparar_paginas(void* a, void* b) {
//      t_pagina* pagina_a = (t_pagina*) a;
//      t_pagina* pagina_b = (t_pagina*) b;
//      return pagina_a->frame < pagina_b->frame;
// }

// void dump_memoria() {
//      FILE* archivo = stdout;
//      // char* tiempo = temporal_get_string_time("Dump_%y%m%d%H%M%S.tlb");
//      // char* nombre = string_from_format("/home/utnso/dumps/memoria/%s", tiempo);
//      // archivo = fopen(nombre, "w");
//      // free(nombre); free(tiempo);
//      // if (archivo == NULL) { printf("No se pudo abrir el archivo del dump\n"); return; }
//      t_list* paginas = list_create();
//      for (int i = 0; i < list_size(carpinchos); i++) {
//           t_carpincho* c = list_get(carpinchos, i);
//           for (int j = 0; j < list_size(c->paginas); j++) {
//                t_pagina* p = list_get(c->paginas, j);
//                if (p->presencia) list_add_sorted(paginas, p, &comparar_paginas);
//           }
//      }
//      fprintf(archivo, "-----------------------------------------------------------------------\n");
//      char* titulo = temporal_get_string_time("Dump: %d/%m/%y %H:%M:%S\n");
//      fprintf(archivo, "%s", titulo);
//      free(titulo);
//      for (int i = 0; i < tamanio_memoria / tamanio_pagina; i++) {
//           if (list_is_empty(paginas)) {
//                fprintf(archivo, "Frame: %d\tCarpincho: -\tPagina: -\n", i);
//                continue;
//           }
//           t_pagina *p = list_get(paginas, 0);
//           if (i == p->frame) {
//                fprintf(archivo, "Frame: %d\tCarpincho: %d\tPagina: %d\n", i, p->pid, p->numero);
//                list_remove(paginas, 0);
//           } else fprintf(archivo, "Frame: %d\tCarpincho: -\tPagina: -\n", i);
//      }
//      fprintf(archivo, "-----------------------------------------------------------------------\n");
//      list_destroy(paginas);
// }