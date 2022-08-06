#include "pagina.h"

t_pagina* reemplazar_pagina_lru(t_list* lista, t_pagina* nueva) {
     // Saco la primera (es la que mas tiempo estuvo)
     t_pagina* sacada = list_remove(lista, 0);
     // Meto la nueva al final
     list_add(lista, nueva);
     return sacada;
}

t_pagina* reemplazar_pagina_clock(t_list* lista, t_pagina* nueva) {
     t_pagina* sacada = NULL;
     // Hago una vuelta buscando (u=0; m=0)
     for (int i = 0; i < list_size(lista); i++) {
          t_pagina *apuntada = list_remove(lista, 0);
          if (!apuntada->modificado && !apuntada->uso) {
               sacada = apuntada;
               list_add(lista, nueva);
               break;
          } else list_add(lista, apuntada);
     }
     if (sacada) return sacada;
     // Si no encontre nada, empiezo a buscar (u=0; m=1), pero voy poniendo u en 1
     while (sacada == NULL) {
          t_pagina* apuntada = list_remove(lista, 0);
          if (!apuntada->uso && apuntada->modificado) {
               sacada = apuntada;
               list_add(lista, nueva);
               break;
          } else {
               apuntada->uso = false;
               list_add(lista, apuntada);
          }
     }
     return sacada;
}

int reemplazar_pagina(t_list* lista, t_pagina* nueva) {
     // Ejecuto el algoritmo de reemplazo
     t_pagina* sacada = algoritmo_mmu == LRU ? reemplazar_pagina_lru(lista, nueva) : reemplazar_pagina_clock(lista, nueva);
     sacada->presencia = false;
     // Si saque una pagina que habia sido modificada, la escribo en swap
     if (sacada->modificado) swap_escribir_pagina(sacada->pid, sacada->numero, memoria_principal + tamanio_pagina * sacada->frame);
     // Devuelvo el frame donde se inserto la pagina
     return sacada->frame;
}

void eliminar_pagina(t_carpincho* carpincho, int numero) {
     // La saco de la lista de paginas del proceso
     t_pagina* pagina = list_remove(carpincho->paginas, numero);
     // Actualizo el numero de las paginas del proceso
     for (int i = numero; i < list_size(carpincho->paginas); i++)
          ((t_pagina*) list_get(carpincho->paginas, i))->numero--;
     // La saco de la lista de pags en memoria (si esta)
     for (int i = 0; i < list_size(carpincho->residentes); i++) 
          if (list_get(carpincho->residentes, i) == pagina) {
               list_remove(carpincho->residentes, i);
               break;
          }
     // Si estaba en la principal, limpio el bit
     if (pagina->presencia) bitarray_clean_bit(bitmap_memoria, pagina->frame);
     // Saco la pagina de la tlb
     sacar_de_tlb(pagina);
     // Libero la memoria que ocupaba
     free(pagina);
}

int proximo_frame(t_carpincho* carpincho) {
     int primero = tipo_asignacion == FIJA ? carpincho->primer_marco : 0;
     int cantidad = tipo_asignacion == FIJA ? marcos_por_carpincho : tamanio_memoria / tamanio_pagina;
     // Recorro el bitarray hasta que encuentro un marco en 0 (free)
     for (int i = primero; i < primero + cantidad; i++)
          if (!bitarray_test_bit(bitmap_memoria, i))
               return i;
     return -1;
}

int buscar_en_principal(t_carpincho* carpincho, int numero) {
     t_pagina* pagina = list_get(carpincho->paginas, numero);
     return pagina->presencia ? pagina->frame : -1;
}

int buscar_pagina(t_carpincho* carpincho, int numero) {
     // Busco la pagina en la tlb
     int frame = buscar_en_tlb(carpincho, numero);
     if (frame != -1) return frame;
     // Si no esta en la tlb, la busco en la principal
     frame = buscar_en_principal(carpincho, numero);
     if (frame != -1) return frame;
     // Si no esta en la tlb ni en la principal, la traigo de la swap
     return traer_pagina(carpincho, numero);
}

void insertar_pagina(t_carpincho* carpincho, t_pagina* pagina, void* contenido) {
     // Pido el proximo frame libre
     int frame = proximo_frame(carpincho);
     // Si no hay frames libres, reemplazo una pagina
     pagina->frame = frame == -1 ? reemplazar_pagina(carpincho->residentes, pagina) : frame;
     // Actualizo los bits
     pagina->presencia = true;
     pagina->uso = true;
     pagina->modificado = false;
     bitarray_set_bit(bitmap_memoria, pagina->frame);
     agregar_a_tlb(pagina);
     // Finalmente, copio el contenido a memoria
     memcpy(memoria_principal + tamanio_pagina * pagina->frame, contenido, tamanio_pagina);
     free(contenido);
}

int traer_pagina(t_carpincho* carpincho, int numero) {
     t_pagina* pagina = list_get(carpincho->paginas, numero);
     // Le pido el contenido a la swap
     void* contenido = swap_leer_pagina(carpincho->pid, numero);
     // La inserto en memoria real
     insertar_pagina(carpincho, pagina, contenido);
     // Devuelvo el marco nuevo
     return pagina->frame;
}