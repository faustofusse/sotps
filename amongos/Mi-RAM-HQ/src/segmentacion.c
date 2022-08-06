#include "segmentacion.h"

void seg_iniciar_variables(void* memoria, u_int32_t tamanio, int alg)
{
     tamanio_memoria = tamanio;
     memoria_principal = memoria;
     algoritmo = alg;
     // Inicializo las tablas
     procesos = list_create();
     huecos = list_create();
     // Empieza con un hueco que empieza en la pos 0 y ocupa toda la memoria
     t_segmento* hueco = malloc(sizeof(t_segmento));
     hueco->inicio = 0;
     hueco->tamanio = tamanio;
     list_add(huecos, hueco);
}

t_list* segmentos_ordenados()
{
     t_list* segmentos = list_create();
     for (int i = 0; i < list_size(procesos); i++){
          t_proceso* proceso = (t_proceso*) list_get(procesos, i);
          for (int j = 0; j < list_size(proceso->segmentos); j++) {
               t_segmento* s = (t_segmento*)list_get(proceso->segmentos, j);
               for (int b = 0; b < list_size(segmentos); b++) {
                    t_segmento* seg = (t_segmento*)list_get(segmentos, b);
                    if (s->inicio < seg->inicio) {
                         list_add_in_index(segmentos, b, s);
                         break;
                    } else if (b == list_size(segmentos) - 1) {
                         list_add(segmentos, s);
                         break;
                    }
               }
               if (list_size(segmentos) == 0) list_add(segmentos, s);
          }
     }
     return segmentos;
}

void compactacion()
{
     t_list* segmentos = segmentos_ordenados();
     // Si hay segmentos, empiezo moviendo el primero
     if (list_size(segmentos) != 0) {
          t_segmento* primero = (t_segmento*) list_get(segmentos, 0);
          if (primero->inicio != 0) {
               memmove(memoria_principal, memoria_principal + primero->inicio, primero->tamanio);
               primero->inicio = 0;
          }
          // Muevo el resto
          for (int i = 1; i < list_size(segmentos); i++) {
               t_segmento* anterior = (t_segmento*) list_get(segmentos, i-1);
               t_segmento* segmento = (t_segmento*) list_get(segmentos, i);
               if (segmento->inicio > anterior->inicio + anterior->tamanio) {
                    memmove(memoria_principal + anterior->inicio + anterior->tamanio, memoria_principal + segmento->inicio, segmento->tamanio);
                    segmento->inicio = anterior->inicio + anterior->tamanio;
               }
          }
     }
     // Saco todos los huecos de la lista
     list_clean_and_destroy_elements(huecos, free);
     // Agrego un unico hueco (si hay espacio en memoria)
     t_segmento* hueco = malloc(sizeof(t_segmento));
     t_segmento* ultimo = list_size(segmentos) == 0 ? NULL : (t_segmento*) list_get(segmentos, list_size(segmentos) - 1);
     if (list_size(segmentos) == 0) {
          hueco->inicio = 0;
          hueco->tamanio = tamanio_memoria;
          list_add(huecos, hueco);
     } else if (ultimo->inicio + ultimo->tamanio < tamanio_memoria) {
          hueco->inicio = ultimo->inicio + ultimo->tamanio;
          hueco->tamanio = tamanio_memoria - hueco->inicio;
          list_add(huecos, hueco);
     }
     list_destroy(segmentos);
}

void* sumar_segmentos(void* a, void* b) {
     u_int32_t* acumulado = (u_int32_t*) a;
     u_int32_t siguiente = ((t_segmento*) b)->tamanio;
     *acumulado += siguiente;
     return acumulado;
}

bool hay_espacio(u_int32_t tamanio_tareas, u_int32_t cantidad_tripulantes)
{
     u_int32_t seed = 0;
     u_int32_t* espacio_libre = (u_int32_t*) list_fold(huecos, &seed, sumar_segmentos);
     u_int32_t espacio = 8 + // PCB
          tamanio_tareas + // Tareas
          21 * cantidad_tripulantes; // TCB
     return *espacio_libre >= espacio;
}

int buscar_hueco(int tamanio) {
     int posicion = -1;
     t_segmento* hueco;
     for (int i = 0; i < list_size(huecos); i++) {
          hueco = list_get(huecos, i);
          // Si el hueco es menor que el segmento, sigo buscando
          if (hueco->tamanio < tamanio) continue;
          // Si el algoritmo es FF, salgo del for
          if (algoritmo == FF) {
               posicion = i;
               break;
          // Si es BF, me fijo si el hueco es mejor que el que estaba
          } else if (posicion == -1 || hueco->tamanio < ((t_segmento*) list_get(huecos, posicion))->tamanio ) {
               posicion = i;
          }
     }
     return posicion;
}

bool hay_algun_hueco_de(int tamanio) {
     for (int i = 0; i < list_size(huecos); i++) {
          t_segmento* hueco = list_get(huecos, i);
          if (hueco->tamanio >= tamanio) return true;
     }
     return false;
}

t_segmento* guardar_segmento(void* contenido, u_int32_t tamanio)
{
     if (!hay_algun_hueco_de(tamanio)) compactacion();
     // Creo el segmento
     t_segmento* nuevo = malloc(sizeof(t_segmento));
     nuevo->tamanio = tamanio;
     // Busco un hueco para meterlo
     int posicion = buscar_hueco(tamanio);
     t_segmento* hueco = list_get(huecos, posicion);
     // Reduzco el hueco. Si tiene el mismo tamanio que el segmento, lo elimino
     int inicio = hueco->inicio;
     if (hueco->tamanio != tamanio) {
          hueco->inicio += tamanio;
          hueco->tamanio -= tamanio;
     } else {
          t_segmento* s = (t_segmento*) list_remove(huecos, posicion);
          free(s);
     }
     // Copio el contenido del nuevo segmento a memoria
     nuevo->inicio = inicio;
     memcpy(memoria_principal + inicio, contenido, tamanio);
     // Libero memoria y retorno el segmento
     free(contenido);
     return nuevo;
}

void* comprimir_tcb(TCB tripulante)
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

bool seg_guardar_patota(u_int32_t pid, char* tareas, int cantidad_tripulantes, char* posiciones_string, int caracter)
{
     int proximo_caracter = caracter;
     sem_wait(&mutex_memoria);
     // Me fijo si entra la patota
     if (!hay_espacio(strlen(tareas), cantidad_tripulantes)) {
          sem_post(&mutex_memoria);
          return false;
     }
     // Creo la tabla del proceso
     t_proceso* proceso = malloc(sizeof(t_proceso));
     proceso->pid = pid;
     t_list* segmentos = list_create();
     proceso->segmentos = segmentos;
     // Agrego el proceso a la tabla de procesos
     list_add(procesos, proceso);
     // Creo el PCB y lo guardo en memoria
     PCB* patota = malloc(sizeof(PCB));
     patota->pid = pid;
     patota->tareas = 0;
     list_add(proceso->segmentos, guardar_segmento((void*) patota, sizeof(PCB)));
     // Separo el string de posiciones
     t_list* posiciones = list_create(); 
     separar_string(posiciones, posiciones_string, ",");
     // Creo los TCB y los guardo en memoria
     for (int i = 0; i < cantidad_tripulantes; i++) {
          TCB tripulante;
          tripulante.tid = i+1;
          tripulante.instruccion = 1;
          tripulante.estado = char_estados[NEW];
          tripulante.x = i > list_size(posiciones) - 1 ? 0 : atoi((char*) list_get(posiciones, i));
          tripulante.y = i > list_size(posiciones) - 1 ? 0 : atoi((char*) list_get(posiciones, i) + 2 * sizeof(char));
          tripulante.PCB = 0;
          void* tcb = comprimir_tcb(tripulante);
          list_add(proceso->segmentos, guardar_segmento(tcb, 21)); // poner el 21 en una constante o algo
          // Lo meto en el mapa
          if (MAPA_ACTIVADO) mapa_agregar_tripulante(pid, tripulante.tid, tripulante.x, tripulante.y, (char) proximo_caracter);
          proximo_caracter++;
     }
     // Guardo las tareas en memoria
     list_add(proceso->segmentos, guardar_segmento((void*) tareas, strlen(tareas)));
     sem_post(&mutex_memoria);
     // Libero memoria
     list_destroy(posiciones);
     return true;
}

void seg_dump()
{
     char* nombre = temporal_get_string_time("./dump/Dump_%y%m%d%H%M%S.dmp");
     FILE* archivo = fopen(nombre, "w");
     free(nombre);
     if (archivo == NULL) { write(STDOUT_FILENO, "No se pudo abrir el archivo del dump\n", 37); return; }
     fwrite("--------------------------------------------------------------------------\n", 1, 75, archivo);
     char* tiempo = temporal_get_string_time("Dump: %d/%m/%y %H:%M:%S\n");
     fwrite(tiempo, 1, strlen(tiempo), archivo);
     free(tiempo);
     t_list* segmentos = list_create();
     t_list* strings = list_create();
     sem_wait(&mutex_memoria);
     for (int i = 0; i < list_size(procesos); i++){
          t_proceso* proceso = (t_proceso*) list_get(procesos, i);
          for (int j = 0; j < list_size(proceso->segmentos); j++) {
               t_segmento* s = (t_segmento*)list_get(proceso->segmentos, j);
               char* string = string_from_format("Proceso: %d \tSegmento: %d \tInicio: %d (0x%04X) \tTamanio: %dB\n", proceso->pid, j+1, s->inicio, s->inicio, s->tamanio);
               if (list_size(segmentos) == 0) list_add(segmentos, string);
               for (int b = 0; b < list_size(segmentos); b++) {
                    t_segmento* seg = (t_segmento*)list_get(segmentos, b);
                    if (s->inicio < seg->inicio) {
                         list_add_in_index(segmentos, b, s);
                         list_add_in_index(strings, b, string);
                         break;
                    }
               }
          }
     }
     for (int i = 0; i < list_size(huecos); i++) {
          t_segmento* hueco = (t_segmento*) list_get(huecos, i);
          char* string = string_from_format("[ Hueco de %d bytes ]\n", hueco->tamanio);
          for (int b = 0; b < list_size(segmentos); b++) {
               t_segmento* seg = (t_segmento*)list_get(segmentos, b);
               if (hueco->inicio < seg->inicio || b == list_size(segmentos) - 1) {
                    list_add_in_index(segmentos, b, hueco);
                    list_add_in_index(strings, b, string);
                    break;
               }
          }
          if (list_size(segmentos) == 0) {
               list_add(segmentos, hueco);
               list_add(strings, string);
          }
     }
     sem_post(&mutex_memoria);
     for (int i = 0; i < list_size(strings); i++) fwrite((char*) list_get(strings, i), 1, strlen((char*) list_get(strings, i)), archivo);
     fwrite("--------------------------------------------------------------------------\n", 1, 75, archivo);
     fclose(archivo);
     list_destroy(segmentos);
     list_destroy_and_destroy_elements(strings, free);
}

t_proceso* buscar_proceso(u_int32_t pid) 
{
     bool condicion(void* p) { 
          return ((t_proceso*)p)->pid == pid;
     };
     t_proceso* proceso = (t_proceso*) list_find(procesos, &condicion);
     return proceso;
}

void sacar_proceso(u_int32_t pid)
{
     bool condicion (void* elemento){
          return ((t_proceso*) elemento)->pid == pid;
     }
     list_remove_by_condition(procesos, &condicion);
}

void eliminar_segmento(t_segmento* segmento)
{
     for (int i = 0; i < list_size(huecos); i++) {
          if (segmento->inicio < ((t_segmento*) list_get(huecos, i))->inicio) {
               list_add_in_index(huecos, i, segmento);
               return;
          }
     }
     list_add(huecos, segmento);
}

void seg_eliminar_pcb(u_int32_t pid)
{
     t_proceso* proceso = buscar_proceso(pid);
     int cantidad_segmentos = list_size(proceso->segmentos);
     for (int i = 0; i < cantidad_segmentos; i++)
          eliminar_segmento((t_segmento*) list_get(proceso->segmentos, i));
     list_clean(proceso->segmentos);
     list_destroy(proceso->segmentos);
     sacar_proceso(pid);
     free(proceso);
}

void seg_eliminar_tcb(u_int32_t pid, u_int32_t tid)
{
     sem_wait(&mutex_memoria);
     t_proceso* proceso = buscar_proceso(pid);
     for (int i = 1; i < list_size(proceso->segmentos) - 1; i++) {
          t_segmento* segmento = (t_segmento*) list_get(proceso->segmentos, i);
          u_int32_t* tid_segmento = malloc(sizeof(u_int32_t));
          memcpy(tid_segmento, memoria_principal + segmento->inicio, sizeof(u_int32_t));
          if (*tid_segmento == tid){
               eliminar_segmento(segmento);
               list_remove(proceso->segmentos, i);
               free(tid_segmento);
               break;
          }
          free(tid_segmento);
     }
     if (list_size(proceso->segmentos) <= 2) seg_eliminar_pcb(pid);
     sem_post(&mutex_memoria);
}

t_segmento* buscar_tcb(u_int32_t pid, u_int32_t tid)
{
     t_proceso* proceso = buscar_proceso(pid);
     t_segmento* segmento;
     u_int32_t* tid_segmento;
     for (int i = 1; i < list_size(proceso->segmentos) - 1; i++) {
          segmento = (t_segmento*) list_get(proceso->segmentos, i);
          tid_segmento = malloc(sizeof(u_int32_t));
          memcpy(tid_segmento, memoria_principal + segmento->inicio, sizeof(u_int32_t));
          if (*tid_segmento == tid) { 
               free(tid_segmento);
               break;
          }
          free(tid_segmento);
     }
     return segmento;
}

u_int32_t siguiente_instruccion(u_int32_t pid, u_int32_t tid)
{
     u_int32_t resultado;
     u_int32_t* instruccion = malloc(sizeof(u_int32_t));
     t_segmento* tcb = buscar_tcb(pid, tid);
     memcpy(instruccion, memoria_principal + tcb->inicio + 13, sizeof(u_int32_t));
     resultado = *instruccion;
     *instruccion = *instruccion + 1;
     memcpy(memoria_principal + tcb->inicio + 13, instruccion, sizeof(u_int32_t));
     free(instruccion);
     return resultado;
}

char* obtener_tareas(u_int32_t pid) 
{
     t_proceso* proceso = buscar_proceso(pid);
     t_segmento* seg_tareas = (t_segmento*) list_get(proceso->segmentos, list_size(proceso->segmentos) - 1);
     char* tareas = malloc(seg_tareas->tamanio + 1);
     memcpy(tareas, memoria_principal + seg_tareas->inicio, seg_tareas->tamanio);
     tareas[seg_tareas->tamanio] = '\0';
     return tareas;
}

char* seg_siguiente_tarea(u_int32_t pid, u_int32_t tid)
{
     char* tarea = NULL;
     sem_wait(&mutex_memoria);
     u_int32_t instruccion = siguiente_instruccion(pid, tid);
     char* tareas = obtener_tareas(pid);
     sem_post(&mutex_memoria);
     // Separo las tareas
     t_list* lista_tareas = list_create();
     separar_string(lista_tareas, tareas, ",");
     // Me fijo si le quedan tareas
     if (instruccion <= (u_int32_t) list_size(lista_tareas)) {
          char* t = list_get(lista_tareas, instruccion - 1);
          tarea = malloc(strlen(t) + 1);
          memcpy(tarea, t, strlen(t));
          tarea[strlen(t)] = '\0';
     }
     list_destroy(lista_tareas);
     free(tareas);
     return tarea;
}

void seg_cambiar_estado(u_int32_t pid, u_int32_t tid, char estado)
{
     sem_wait(&mutex_memoria);
     t_segmento* tcb = buscar_tcb(pid, tid);
     memcpy(memoria_principal + tcb->inicio + 4, &estado, sizeof(char));
     sem_post(&mutex_memoria);
}

void seg_mover(u_int32_t pid, u_int32_t tid, u_int32_t x, u_int32_t y)
{
     sem_wait(&mutex_memoria);
     t_segmento* tcb = buscar_tcb(pid, tid);
     memcpy(memoria_principal + tcb->inicio + 5, &x, sizeof(u_int32_t));
     memcpy(memoria_principal + tcb->inicio + 9, &y, sizeof(u_int32_t));
     sem_post(&mutex_memoria);
}