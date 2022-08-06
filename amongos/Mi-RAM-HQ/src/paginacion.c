#include "paginacion.h"

int min(int a, int b) { return a > b ? b : a; }
int max(int a, int b) { return a > b ? a : b; }

void adelantar_puntero() {
     puntero++;
     if (puntero >= list_size(paginas_memoria)) puntero = 0;
}

void pag_iniciar_variables(void* memoria, int tamanio, int alg_reemplazo, char* path_swap, int tamanio_pagina, int swap)
{
     // Inicializo variables globales
     memoria_principal = memoria;
     tamanio_memoria = tamanio;
     algoritmo_reemplazo = alg_reemplazo;
     tamanio_pag = tamanio_pagina;
     tamanio_swap = swap;
     puntero = 0;
     // Creo la lista de procesos vacia
     procesos = list_create();
     paginas_memoria = list_create();
     // Inicializo los bitarray en 0
     int tamanio_bitarray = max(tamanio / tamanio_pag / 8, (tamanio / tamanio_pag / 8) + 1);
     void* bits = malloc(tamanio_bitarray);
     frames_real = bitarray_create(bits, tamanio_bitarray);
     for (int i = 0; i < tamanio / tamanio_pag; i++) bitarray_clean_bit(frames_real, i);
     tamanio_bitarray = max(tamanio_swap / tamanio_pag / 8, (tamanio_swap / tamanio_pag / 8) + 1);
     void* bits_swap = malloc(tamanio_bitarray);
     frames_swap = bitarray_create(bits_swap, tamanio_bitarray);
     for (int i = 0; i < tamanio_swap / tamanio_pag; i++) bitarray_clean_bit(frames_swap, i);
     // Abro archivo de swap y lo agrando
     int archivo;
     verificar( archivo = open(path_swap, O_RDWR | O_CREAT, 0777) , "Error al abrir el archivo");
     verificar( truncate(path_swap, tamanio_swap) , "No se pudo cambiar el tamanio del archivo");
     // Mapeo
     memoria_swap = mmap(NULL, tamanio_swap, PROT_WRITE | PROT_READ, MAP_SHARED, archivo, 0);
}

bool hay_espacio_pag(int espacio_necesitado)
{
     int pags_necesitadas = espacio_necesitado % tamanio_pag == 0 ? espacio_necesitado / tamanio_pag : espacio_necesitado / tamanio_pag + 1;
     int pags_libres = 0;
     for (int i = 0; i < tamanio_memoria / tamanio_pag; i++) {
          if (!bitarray_test_bit(frames_real, i)) pags_libres++;
          if (pags_libres >= pags_necesitadas) return true;
     }
     for (int i = 0; i < tamanio_swap / tamanio_pag; i++) {
          if (!bitarray_test_bit(frames_swap, i)) pags_libres++;
          if (pags_libres >= pags_necesitadas) return true;
     }
     return false;
}

void* comprimir_tcb2(TCB tripulante) // SE REPITE EN segmentacion.c !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
{
     void* tcb = malloc(21);
     memcpy(tcb, &tripulante.tid, sizeof(u_int32_t));
     memcpy(tcb + sizeof(u_int32_t), &tripulante.estado, sizeof(char));
     memcpy(tcb + sizeof(u_int32_t) + sizeof(char), &tripulante.x, sizeof(u_int32_t));
     memcpy(tcb + 2 * sizeof(u_int32_t) + sizeof(char), &tripulante.y, sizeof(u_int32_t));
     memcpy(tcb + 3 * sizeof(u_int32_t) + sizeof(char), &tripulante.instruccion, sizeof(u_int32_t));
     memcpy(tcb + 4 * sizeof(u_int32_t) + sizeof(char), &tripulante.PCB, sizeof(u_int32_t));
     return tcb;
}

int proximo_frame(t_bitarray* array, int size) {
     for (int i = 0; i < size; i++) if (!bitarray_test_bit(array, i)) return i;
     return -1;
}

// ESTA FUNCION ES MUY PARECIDA A traer_pagina!
int insertar_pagina(t_pagina* pagina) {
     t_pagina* sacada;
     if (algoritmo_reemplazo == LRU) { 
          // Si es LRU, saco la primera (es la que mas tiempo estuvo sin usar), y meto la nueva al final
          sacada = (t_pagina*) list_remove(paginas_memoria, 0);
          list_add(paginas_memoria, pagina);
     } else if (algoritmo_reemplazo == CLOCK) {
          // Si es CLOCK, empiezo por la apuntada por el puntero y me voy fijando si el bit de uso esta en 0
          t_pagina* apuntada = (t_pagina*) list_get(paginas_memoria, puntero);
          while (apuntada->uso) {
               apuntada->uso = 0;
               adelantar_puntero();
               apuntada = (t_pagina*) list_get(paginas_memoria, puntero);
          }
          // Cambio la vieja x la nueva
          sacada = (t_pagina*) list_remove(paginas_memoria, puntero);
          list_add_in_index(paginas_memoria, puntero, pagina);
          adelantar_puntero();
     }
     sacada->presencia = 0;
     pagina->frame = sacada->frame;
     // Meto la que saque de la principal, en la memoria virtual
     int frame_virtual = proximo_frame(frames_swap, tamanio_swap / tamanio_pag);
     memcpy(memoria_swap + frame_virtual * tamanio_pag, memoria_principal + sacada->frame * tamanio_pag, tamanio_pag);
     sacada->frame = frame_virtual;
     // Actualizo el bitarray
     bitarray_set_bit(frames_swap, frame_virtual);
     return pagina->frame;
}

void guardar_paginas(void* contenido, int tamanio, t_list* paginas)
{
     int cantidad_paginas = tamanio % tamanio_pag == 0 ? tamanio / tamanio_pag : tamanio / tamanio_pag + 1;
     int tamanio_ultima_pag = tamanio % tamanio_pag == 0 ? tamanio_pag : tamanio - (tamanio_pag * (cantidad_paginas - 1));
     int i, frame, cantidad_frames = tamanio_memoria / tamanio_pag;
     t_pagina* pagina;
     for (i = 0; i < cantidad_paginas; i++) {
          pagina = malloc(sizeof(t_pagina));
          pagina->presencia = 1;
          pagina->uso = 1;
          // Busco el proximo marco libre
          frame = proximo_frame(frames_real, cantidad_frames);
          pagina->frame = frame;
          // Si se acaba el lugar en la real, paso una a swap
          if (frame == -1) frame = insertar_pagina(pagina);
          // Copio contenido al marco
          int tamanio_a_copiar = i == cantidad_paginas - 1 ? tamanio_ultima_pag : tamanio_pag;
          memcpy(memoria_principal + frame * tamanio_pag, contenido + i * tamanio_pag, tamanio_a_copiar);
          bitarray_set_bit(frames_real, frame);
          // Agrego la pagina a las listas
          list_add(paginas, pagina);
     }
     free(contenido);
}

// void guardar_paginas(void* contenido, int tamanio, t_list* paginas)
// {
//      int cantidad_paginas = tamanio % tamanio_pag == 0 ? tamanio / tamanio_pag : tamanio / tamanio_pag + 1;
//      int tamanio_ultima_pag = tamanio % tamanio_pag == 0 ? tamanio_pag : tamanio - (tamanio_pag * (cantidad_paginas - 1));
//      // Para empezar, asumo que hay lugar en la memoria real
//      t_bitarray* frames = frames_real;
//      void* memoria = memoria_principal;
//      int i, frame, cantidad_frames = tamanio_memoria / tamanio_pag;
//      t_pagina* pagina;
//      for (i = 0; i < cantidad_paginas; i++) {
//           pagina = malloc(sizeof(t_pagina));
//           pagina->presencia = memoria == memoria_principal;
//           pagina->uso = 1;
//           // Busco el proximo marco libre
//           frame = proximo_frame(frames, cantidad_frames);
//           // Si se acaba el lugar en la real, paso a la swap
//           if (frame == -1) {
//                pagina->presencia = 0;
//                frames = frames_swap;
//                memoria = memoria_swap;
//                cantidad_frames = tamanio_swap / tamanio_pag;
//                frame = proximo_frame(frames, cantidad_frames);
//           }
//           // Copio contenido al marco
//           int tamanio_a_copiar = i == cantidad_paginas - 1 ? tamanio_ultima_pag : tamanio_pag;
//           memcpy(memoria + frame * tamanio_pag, contenido + i * tamanio_pag, tamanio_a_copiar);
//           bitarray_set_bit(pagina->presencia == 0 ? frames_swap : frames_real, frame);
//           // Agrego la pagina a las listas
//           pagina->frame = frame;
//           list_add(paginas, pagina);
//           if(pagina->presencia) list_add(paginas_memoria, pagina);
//      }
//      free(contenido);
// }

bool pag_guardar_patota(u_int32_t pid, char* tareas, int cantidad_tripulantes, char* posiciones_string, int caracter)
{
     int tamanio_proceso = 2 * sizeof(u_int32_t) + strlen(tareas) + cantidad_tripulantes * 21;
     int proximo_caracter = caracter;

     sem_wait(&mutex_memoria);

     // Me fijo si hay espacio
     if (!hay_espacio_pag(tamanio_proceso)) {
          sem_post(&mutex_memoria);
          return false;
     }
     // Creo la tabla del proceso
     p_proceso* proceso = malloc(sizeof(p_proceso));
     proceso->pid = pid;
     proceso->tamanio = tamanio_proceso;
     proceso->paginas = list_create();
     proceso->tcbs = list_create();
     // Creo el contenido a pasar a memoria
     void* contenido = malloc(tamanio_proceso);
     u_int32_t direccion_tareas = 8 + cantidad_tripulantes * 21;
     u_int32_t direccion_pcb = 0;
     // Copio el PCB
     memcpy(contenido, &pid, sizeof(u_int32_t));
     memcpy(contenido + sizeof(u_int32_t), &direccion_tareas, sizeof(u_int32_t));
     // Separo el string de posiciones
     t_list* posiciones = list_create(); 
     separar_string(posiciones, posiciones_string, ",");
     // Creo los TCB y los copio a contenido
     for (u_int32_t i = 0; i < cantidad_tripulantes; i++) {
          u_int32_t* numero = malloc(sizeof(u_int32_t));
          memcpy(numero, &i, sizeof(u_int32_t));
          list_add(proceso->tcbs, numero);
          TCB tripulante;
          tripulante.tid = i + 1;
          tripulante.instruccion = direccion_tareas;
          tripulante.estado = char_estados[NEW];
          tripulante.x = i > list_size(posiciones) - 1 ? 0 : atoi((char*) list_get(posiciones, i));
          tripulante.y = i > list_size(posiciones) - 1 ? 0 : atoi((char*) list_get(posiciones, i) + 2 * sizeof(char));
          tripulante.PCB = direccion_pcb;
          void* tcb = comprimir_tcb2(tripulante);
          memcpy(contenido + 2 * sizeof(u_int32_t) + i * 21, tcb, 21);
          free(tcb);
          // Lo meto en el mapa
          if (MAPA_ACTIVADO) mapa_agregar_tripulante(pid, tripulante.tid, tripulante.x, tripulante.y, (char) proximo_caracter);
          proximo_caracter++;
     }
     // Copio las tareas
     memcpy(contenido + 2 * sizeof(u_int32_t) + 21 * cantidad_tripulantes, tareas, strlen(tareas));
     // Guardo las paginas en la memoria y agrego el proceso a la tabla
     guardar_paginas(contenido, tamanio_proceso, proceso->paginas);
     list_add(procesos, proceso);

     sem_post(&mutex_memoria);

     // Libero memoria
     list_destroy(posiciones);
     free(tareas);

     return true;
}

p_proceso* pag_buscar_proceso(u_int32_t pid)
{
     for (int i = 0; i < list_size(procesos); i++) {
          p_proceso* proceso = (p_proceso*) list_get(procesos, i);
          if (proceso->pid == pid) return proceso;
     }
     return NULL;
}

int obtener_posicion(t_pagina* pagina)
{
     for (int i = 0; i < list_size(paginas_memoria); i++) 
          if (pagina == (t_pagina*) list_get(paginas_memoria, i)) 
               return i;
     return -1;
}

void traer_pagina(t_pagina* pagina)
{
     t_pagina* sacada;
     pagina->presencia = 1;
     pagina->uso = 1;
     bitarray_clean_bit(frames_swap, pagina->frame);
     // Me fijo si hay algun marco libre
     int frame = proximo_frame(frames_real, tamanio_memoria / tamanio_pag);
     if (frame != -1) {
          memcpy(memoria_principal + frame * tamanio_pag, memoria_swap + pagina->frame * tamanio_pag, tamanio_pag);
          bitarray_set_bit(frames_real, frame);
          pagina->frame = frame;
          list_add(paginas_memoria, pagina);
          return;
     } else if (algoritmo_reemplazo == LRU) { 
          // Si es LRU, saco la primera (es la que mas tiempo estuvo sin usar), y meto la nueva al final
          sacada = (t_pagina*) list_remove(paginas_memoria, 0);
          list_add(paginas_memoria, pagina);
     } else if (algoritmo_reemplazo == CLOCK) {
          // Si es CLOCK, empiezo por la apuntada por el puntero y me voy fijando si el bit de uso esta en 0
          t_pagina* apuntada = (t_pagina*) list_get(paginas_memoria, puntero);
          while (apuntada->uso) {
               apuntada->uso = 0;
               adelantar_puntero();
               apuntada = (t_pagina*) list_get(paginas_memoria, puntero);
          }
          // Cambio la vieja x la nueva
          sacada = (t_pagina*) list_remove(paginas_memoria, puntero);
          list_add_in_index(paginas_memoria, puntero, pagina);
          adelantar_puntero();
     }
     sacada->presencia = 0;
     frame = sacada->frame;
     // Pongo el contenido de la que saque en una variable temporal
     void* temp = malloc(tamanio_pag);
     memcpy(temp, memoria_principal + sacada->frame * tamanio_pag, tamanio_pag);
     // Paso la pagina que estaba en memoria virtual a memoria real
     memcpy(memoria_principal + sacada->frame * tamanio_pag, memoria_swap + pagina->frame * tamanio_pag, tamanio_pag);
     pagina->frame = frame;
     // Meto la que saque de la principal, en la memoria virtual
     int frame_virtual = proximo_frame(frames_swap, tamanio_swap / tamanio_pag);
     sacada->frame = frame_virtual;
     memcpy(memoria_swap + frame_virtual * tamanio_pag, temp, tamanio_pag);
     // Actualizo el bitarray y libero la variable temporal
     bitarray_set_bit(frames_swap, frame_virtual);
     free(temp);
}

void usar_pagina(t_pagina* pagina)
{
     pagina->uso = 1;
     if (algoritmo_reemplazo == LRU) { 
          int i = obtener_posicion(pagina);
          list_remove(paginas_memoria, i);
          list_add(paginas_memoria, pagina);
     }
}

int paginas_tcb(int tid, int* cant, int* offs) {
     int direccion_tcb = 8 + 21 * (tid - 1);
     int tamanio = 21;
     int cantidad = 1;
     int pagina = direccion_tcb / tamanio_pag;
     int offset = direccion_tcb % tamanio_pag;
     int sobrante = tamanio - (tamanio_pag - offset);
     if (sobrante > 0) cantidad += sobrante % tamanio_pag == 0 ? sobrante / tamanio_pag : sobrante / tamanio_pag + 1;
     if (offs != NULL) *offs = offset;
     if (cant != NULL) *cant = cantidad;
     return pagina;
}

void* copiar_tcb(p_proceso* proceso, u_int32_t tid)
{
     // Calculo de direcciones
     int cant_pags = 0;
     int offset = 0;
     int pagina = paginas_tcb(tid, &cant_pags, &offset);
     // Traigo las paginas a memoria principal, y voy copiando su contenido
     void* contenido = malloc(21);
     int desplazamiento = 0;
     for (int i = pagina; i < pagina + cant_pags; i++){
          t_pagina* p = (t_pagina*) list_get(proceso->paginas, i);
          if (!(p->presencia)) traer_pagina(p); else usar_pagina(p);
          int a_copiar = min(21 - desplazamiento, tamanio_pag - offset);
          memcpy(contenido + desplazamiento, memoria_principal + p->frame * tamanio_pag + offset, a_copiar);
          desplazamiento += a_copiar;
          offset = 0;
     }
     return contenido;
}

u_int32_t pag_siguiente_instruccion(p_proceso* proceso, u_int32_t tid)
{
     void* tcb = copiar_tcb(proceso, tid);
     // Extraigo la proxima instruccion
     u_int32_t proxima;
     memcpy(&proxima, tcb + sizeof(u_int32_t) * 3 + sizeof(char), sizeof(u_int32_t));
     free(tcb);
     return proxima;
}

void editar_proceso(p_proceso* proceso, int direccion, int tamanio, void* contenido)
{
     // Calculo de direcciones
     int cant_pags = 1;
     int pagina = direccion / tamanio_pag;
     int offset = direccion % tamanio_pag;
     int sobrante = tamanio - (tamanio_pag - offset);
     if (sobrante > 0) cant_pags += sobrante % tamanio_pag == 0 ? sobrante / tamanio_pag : sobrante / tamanio_pag + 1;
     // Traigo las paginas a memoria principal, y voy copiando
     int desplazamiento = 0;
     for (int i = pagina; i < pagina + cant_pags; i++){
          t_pagina* p = (t_pagina*) list_get(proceso->paginas, i);
          if (!(p->presencia)) traer_pagina(p); else usar_pagina(p);
          int a_copiar = min(tamanio - desplazamiento, tamanio_pag - offset);
          memcpy(memoria_principal + p->frame * tamanio_pag + offset, contenido + desplazamiento, a_copiar);
          desplazamiento += a_copiar;
          offset = 0;
     }
}

char* pag_siguiente_tarea(u_int32_t pid, u_int32_t tid) {
     sem_wait(&mutex_memoria);
     p_proceso* proceso = pag_buscar_proceso(pid);
     u_int32_t instruccion = pag_siguiente_instruccion(proceso, tid);
     if (instruccion >= proceso->tamanio) {
          sem_post(&mutex_memoria);
          return NULL;
     }
     int pagina = instruccion / tamanio_pag;
     int offset = instruccion % tamanio_pag;
     int tamanio_tarea = 0;
     char* tarea = NULL;
     char caracter = 0;
     while (instruccion < proceso->tamanio && caracter != ',') {
          tamanio_tarea++;
          tarea = realloc(tarea, tamanio_tarea);
          pagina = instruccion / tamanio_pag;
          offset = instruccion % tamanio_pag;
          t_pagina* p = (t_pagina*) list_get(proceso->paginas, pagina);
          if (!(p->presencia)) traer_pagina(p); else usar_pagina(p);
          memcpy(&caracter, memoria_principal + p->frame * tamanio_pag + offset, sizeof(char));
          tarea[tamanio_tarea - 1] = caracter == ',' ? '\0' : caracter;
          instruccion++;
     }
     if (instruccion >= proceso->tamanio) {
          tamanio_tarea++;
          tarea = realloc(tarea, tamanio_tarea);
          tarea[tamanio_tarea - 1] = '\0';
     }
     // Actualizo la instruccion en el TCB
     int direccion_instruccion = 8 + (tid - 1) * 21 + 3 * sizeof(u_int32_t) + sizeof(char);
     editar_proceso(proceso, direccion_instruccion, sizeof(u_int32_t), &instruccion);
     sem_post(&mutex_memoria);
     return tarea;
}

void pag_cambiar_estado(u_int32_t pid, u_int32_t tid, char estado)
{
     sem_wait(&mutex_memoria);
     p_proceso* proceso = pag_buscar_proceso(pid);
     int direccion = 8 + (tid - 1) * 21 + sizeof(u_int32_t);
     editar_proceso(proceso, direccion, sizeof(char), &estado);
     sem_post(&mutex_memoria);
}

void pag_mover(u_int32_t pid, u_int32_t tid, u_int32_t x, u_int32_t y)
{
     sem_wait(&mutex_memoria);
     void* contenido = malloc(2 * sizeof(u_int32_t));
     memcpy(contenido, &x, sizeof(u_int32_t));
     memcpy(contenido + sizeof(u_int32_t), &y, sizeof(u_int32_t));
     p_proceso* proceso = pag_buscar_proceso(pid);
     int direccion = 8 + (tid - 1) * 21 + sizeof(u_int32_t) + sizeof(char);
     editar_proceso(proceso, direccion, 2 * sizeof(u_int32_t), contenido);
     sem_post(&mutex_memoria);
     free(contenido);
}

void sacar_pag_de_memoria_real(t_pagina* pagina)
{
     for (int j = 0; j < list_size(paginas_memoria); j++) {
          t_pagina* p = (t_pagina*) list_get(paginas_memoria, j);
          if (p != pagina) continue;
          list_remove(paginas_memoria, j);
          if (puntero >= list_size(paginas_memoria)) puntero = 0;
     }
}

void pag_eliminar_pcb(u_int32_t pid)
{
     int i;
     for (i = 0; i < list_size(procesos); i++) if (((p_proceso*) list_get(procesos, i))->pid == pid) break;
     p_proceso* proceso = (p_proceso*) list_remove(procesos, i);
     for (i = 0; i < list_size(proceso->paginas); i++) {
          t_pagina* pagina = (t_pagina*) list_get(proceso->paginas, i);
          bitarray_clean_bit(pagina->presencia ? frames_real : frames_swap, pagina->frame);
          if (!pagina->presencia) continue;
          sacar_pag_de_memoria_real(pagina);
     }
     list_destroy_and_destroy_elements(proceso->paginas, free);
     list_destroy_and_destroy_elements(proceso->tcbs, free);
     free(proceso);
}

void sacar_tcb(t_list* tcbs, u_int32_t numero)
{
     for (int i = 0; i < list_size(tcbs); i++) {
          u_int32_t* n = (u_int32_t*) list_get(tcbs, i);
          if (*n != numero) continue;
          u_int32_t* sacado = (u_int32_t*) list_remove(tcbs, i);
          free(sacado);
     }
}

u_int32_t direccion_tareas(p_proceso* proceso) {
     u_int32_t direccion_tareas = 0;
     int direccion = sizeof(u_int32_t);
     // Calculo de direcciones
     int cant_pags = 1;
     int pagina = direccion / tamanio_pag;
     int offset = direccion % tamanio_pag;
     int sobrante = sizeof(u_int32_t) - (tamanio_pag - offset);
     if (sobrante > 0) cant_pags += sobrante % tamanio_pag == 0 ? sobrante / tamanio_pag : sobrante / tamanio_pag + 1;
     // Traigo las paginas a memoria principal, y voy copiando
     int desplazamiento = 0;
     for (int i = pagina; i < pagina + cant_pags; i++){
          t_pagina* p = (t_pagina*) list_get(proceso->paginas, i);
          if (!(p->presencia)) traer_pagina(p); else usar_pagina(p);
          int a_copiar = min(sizeof(u_int32_t) - desplazamiento, tamanio_pag - offset);
          memcpy(&direccion_tareas + desplazamiento, memoria_principal + p->frame * tamanio_pag + offset, a_copiar);
          desplazamiento += a_copiar;
          offset = 0;
     }
     return direccion_tareas;
}

void pag_eliminar_tcb(u_int32_t pid, u_int32_t tid)
{
     sem_wait(&mutex_memoria);

     p_proceso* proceso = pag_buscar_proceso(pid);
     sacar_tcb(proceso->tcbs, tid - 1);

     if (list_size(proceso->tcbs) == 0) {
          // Si era el ultimo tcb, directamente elimino todo el proceso
          pag_eliminar_pcb(pid);
     } else {
          t_list* borrar = list_create();

          int cantidad = 0; // Cantidad de paginas que se borran
          int pagina = paginas_tcb(tid, &cantidad, NULL); // Pagina desde donde se empieza a borrar

          int primera_pag_tareas = direccion_tareas(proceso) / tamanio_pag;

          for (int i = max(1, pagina); i < min(pagina + cantidad, primera_pag_tareas); i++) {
               int* numero = malloc(sizeof(int));
               memcpy(numero, &i, sizeof(int));
               list_add(borrar, numero);
          }

          for (int i = 0; i < list_size(borrar); i++) log_info(logger, "(PAGINACION) Por ahora borro la pag %d", *((int*) list_get(borrar, i)));

          // Me fijo si hay tcbs que comparten esas paginas
          for (int i = 0; i < list_size(proceso->tcbs); i++) {
               int id = *((int*) list_get(proceso->tcbs, i));
               pagina = paginas_tcb(id + 1, &cantidad, NULL);
               log_info(logger, "(PAGINACION) Checkeo el tcb %d, que esta en la pag %d (cant = %d)", id + 1, pagina, cantidad);
               for (int j = max(1, pagina); j < pagina + cantidad; j++) {
                    for (int b = 0; b < list_size(borrar); b++) {
                         if (j == *((int*) list_get(borrar, b))) {
                              list_remove_and_destroy_element(borrar, b, free);
                              break;
                         }
                    }
               }
          }

          for (int i = 0; i < list_size(borrar); i++) log_info(logger, "(PAGINACION) Borro la pag %d", *((int*) list_get(borrar, i)));
          for (int i = 0; i < list_size(borrar); i++) {
               t_pagina* p = (t_pagina*) list_get(proceso->paginas, *((int*) list_get(borrar, i)));
               if (p->frame == -1) continue;
               bitarray_clean_bit(p->presencia ? frames_real : frames_swap, p->frame);
               if (p->presencia) sacar_pag_de_memoria_real(p);
               p->frame = -1;
          }

          list_destroy_and_destroy_elements(borrar, free);
     }

     sem_post(&mutex_memoria);
}

void pag_dump(bool swap)
{
     char* nombre = temporal_get_string_time("./dump/Dump_%y%m%d%H%M%S.dmp");
     FILE* archivo = fopen(nombre, "w");
     free(nombre);
     if (archivo == NULL) { write(STDOUT_FILENO, "No se pudo abrir el archivo del dump\n", 37); return; }
     fwrite("--------------------------------------------------------------------------\n", 1, 75, archivo);
     char* tiempo = temporal_get_string_time("Dump: %d/%m/%y %H:%M:%S\n");
     fwrite(tiempo, 1, strlen(tiempo), archivo);
     free(tiempo);
     t_list* paginas = list_create();
     t_list* strings = list_create();
     sem_wait(&mutex_memoria);

     for (int i = 0; i < list_size(procesos); i++) {
          p_proceso* proceso = (p_proceso*) list_get(procesos, i);
          for (int j = 0; j < list_size(proceso->paginas); j++) {
               t_pagina* pagina = (t_pagina*) list_get(proceso->paginas, j);
               if (swap == pagina->presencia) continue;
               if (pagina->frame == -1) continue;
               char* string = string_from_format("Marco: %d\t\tEstado: Ocupado\tProceso: %d \tPagina: %d\n", pagina->frame, proceso->pid, j+1);
               int b, index = 0;
               for (b = 0; b < list_size(paginas); b++) {
                    t_pagina* actual = (t_pagina*) list_get(paginas, b);
                    if (pagina->frame < actual->frame) {
                         index = b;
                         break;
                    } else if (b == list_size(paginas) - 1)
                         index = b + 1;
               }
               list_add_in_index(paginas, index, pagina);
               list_add_in_index(strings, index, string);
          }
     }
     int actual = 0;
     t_pagina* pagina = list_size(paginas) > 0 ? (t_pagina*) list_get(paginas, 0) : NULL;
     for (int i = 0; i < (swap ? tamanio_swap : tamanio_memoria) / tamanio_pag; i++) {
          if (pagina == NULL || pagina->frame > i) {
               char* string = string_from_format("Marco: %d\t\tEstado: Libre  \tProceso: - \tPagina: -\n", i);
               fwrite(string, 1, strlen(string), archivo);
               free(string);
          } else {
               char* string = (char*) list_get(strings, actual);
               fwrite(string, 1, strlen(string), archivo);
               actual++;
               if (actual >= list_size(paginas)) pagina = NULL;
               else pagina = (t_pagina*) list_get(paginas, actual);
          }
     }

     sem_post(&mutex_memoria);
     fwrite("--------------------------------------------------------------------------\n", 1, 75, archivo);
     fclose(archivo);
     list_destroy(paginas);
     list_destroy_and_destroy_elements(strings, free);
     // if (!swap) { sleep(1); pag_dump(true);}
}