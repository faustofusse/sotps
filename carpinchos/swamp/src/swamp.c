#include "swamp.h"

void get_config(t_config* config) {
     tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP");
     tamanio_pagina = config_get_int_value(config, "TAMANIO_PAGINA");
     marcos_por_carpincho = config_get_int_value(config, "MARCOS_MAXIMOS");
     retardo = config_get_int_value(config, "RETARDO_SWAP");
}

void iniciar_archivos(char** path_archivos) {
     int cant_paginas = tamanio_swap / tamanio_pagina;
     int tamanio_bitarray = cant_paginas / 8;
     archivos = list_create();
     bitmaps = list_create();
     for (int i = 0; path_archivos[i] != NULL; i++) {
          // Archivo
          int archivo = open(path_archivos[i], O_RDWR | O_CREAT, 0666);
          ftruncate(archivo, tamanio_swap);
          void* contenido = mmap(NULL, tamanio_swap, PROT_READ | PROT_WRITE, MAP_SHARED, archivo, 0);
          // for (int j = 0; j < tamanio_swap; j++) contenido[j] = '\0';
          memset(contenido, '\0', tamanio_swap);
          list_add(archivos, contenido);
          // Bitarray
          void* bits = malloc(tamanio_bitarray);
          t_bitarray* bitarray = bitarray_create(bits, cant_paginas);
          for (int j = 0; j < cant_paginas; j++) bitarray_clean_bit(bitarray, j);
          list_add(bitmaps, bitarray);
          // Libero memoria
          free(path_archivos[i]);
     }
     free(path_archivos);
     printf("Se iniciaron los archivos!\n");
}

int tamanio_archivo(t_bitarray* bitmap) {
     int marcos_libres = 0;
     for (int i = 0; i < tamanio_swap / tamanio_pagina; i++)
          if (!bitarray_test_bit(bitmap, i)) marcos_libres++;
     return marcos_libres;
}

int archivo_con_mas_espacio() {
     int i = 0;
     int max = tamanio_archivo(list_get(bitmaps, 0));
     for (int j = 1; j < list_size(bitmaps); j++) {
          int tamanio = tamanio_archivo(list_get(bitmaps, j));
          if (tamanio > max) {
               max = tamanio;
               i = j;
          }
     }
     return i;
}

int32_t iniciar_carpincho(int pid) {
     // Para ambas asignaciones
     t_carpincho* carpincho = malloc(sizeof(t_carpincho));
     carpincho->pid = pid;
     carpincho->archivo = -1;
     carpincho->paginas = list_create();
     if (asignacion == DINAMICA) return list_add(carpinchos, carpincho);
     // Para asignacion FIJA:
     carpincho->archivo = archivo_con_mas_espacio();
     t_bitarray* bitmap = list_get(bitmaps, carpincho->archivo);
     if (tamanio_archivo(bitmap) < marcos_por_carpincho) {
          log_error(logger, "No hay espacio suficiente en la SWAP");
          free(carpincho);
          return -1;
     }
     int primera = 0;
     while (bitarray_test_bit(bitmap, primera)) primera++;
     for (int i = 0; i < marcos_por_carpincho; i++) {
          bitarray_set_bit(bitmap, primera + i);
          t_pagina* pagina = malloc(sizeof(t_pagina));
          pagina->marco = primera + i;
          pagina->libre = true;
          list_add(carpincho->paginas, pagina);
     }
     return list_add(carpinchos, carpincho);
}

void destruir_carpincho(int pid) {
     for (int i = 0; i < list_size(carpinchos); i++) {
          t_carpincho* carpincho = list_get(carpinchos, i);
          if (carpincho->pid == pid) {
               t_bitarray* bitmap = list_get(bitmaps, carpincho->archivo);
               for (int j = 0; j < list_size(carpincho->paginas); j++) {
                    t_pagina* pagina = list_get(carpincho->paginas, j);
                    bitarray_clean_bit(bitmap, pagina->marco);
                    memset(list_get(archivos, carpincho->archivo) + pagina->marco * tamanio_pagina, 0, tamanio_pagina);
                    free(pagina);
               }
               list_destroy(carpincho->paginas);
               free(carpincho);
               list_remove(carpinchos, i);
               return;
          }
     }
}

t_config* crear_config() {
     return config_create("./config.cfg");
}

t_log* crear_logger() {
     return log_create("./swamp.log", "SWAmP", 1, LOG_LEVEL_INFO);
}

t_carpincho* buscar_carpincho(int pid) {
     for (int i = 0; i < list_size(carpinchos); i++) {
          t_carpincho* carpincho = list_get(carpinchos, i);
          if (carpincho->pid == pid) return carpincho;
     }
     return NULL;
}

int agregar_pagina(t_carpincho* carpincho) {
     t_bitarray* bitmap = list_get(bitmaps, carpincho->archivo);
     int i;
     for (i = 0; bitarray_test_bit(bitmap, i); i++)
          if (i == bitarray_get_max_bit(bitmap))
               return -1;
     bitarray_set_bit(bitmap, i);
     t_pagina* pagina = malloc(sizeof(t_pagina));
     pagina->marco = i;
     pagina->libre = false;
     list_add(carpincho->paginas, pagina);
     return i;
}

int escribir_pagina(int pid, int pagina, void* contenido) {
     printf("Escribiendo en la pagina %d del proceso %d\n", pagina, pid);
     t_carpincho* carpincho = buscar_carpincho(pid);
     if (carpincho == NULL) return -1;
     int marco = -1;
     if (asignacion == DINAMICA) {
          if (list_is_empty(carpincho->paginas)) carpincho->archivo = archivo_con_mas_espacio();
          if (pagina == list_size(carpincho->paginas))
               marco = agregar_pagina(carpincho);
          else if (pagina < list_size(carpincho->paginas))
               marco = ((t_pagina *)list_get(carpincho->paginas, pagina))->marco;
          else return -1;
     } else if (asignacion == FIJA) {
          for (int i = 0; i < list_size(carpincho->paginas); i++) {
               t_pagina* pagina = list_get(carpincho->paginas, i);
               if (pagina->libre) {
                    marco = pagina->marco;
                    break;
               }
          }
     }
     if (marco == -1) return -1;
     ((t_pagina *) list_get(carpincho->paginas, pagina))->libre = false;
     void* archivo = list_get(archivos, carpincho->archivo);
     memcpy(archivo + marco * tamanio_pagina, contenido, tamanio_pagina);
     dump_swap();
     return 0;
}

void* leer_pagina(int pid, int numero) {
     t_carpincho* carpincho = buscar_carpincho(pid);
     if (carpincho == NULL) return NULL;
     if (numero >= list_size(carpincho->paginas)) return NULL;
     t_pagina* pagina = list_get(carpincho->paginas, numero);
     if (pagina->libre) return NULL;
     void* archivo = list_get(archivos, carpincho->archivo);
     void* contenido = malloc(tamanio_pagina);
     memcpy(contenido, archivo + pagina->marco * tamanio_pagina, tamanio_pagina);
     return contenido;
}

int eliminar_pagina(int pid, int numero) {
     t_carpincho* carpincho = buscar_carpincho(pid);
     if (carpincho == NULL) return -1;
     if (numero >= list_size(carpincho->paginas)) return -1;
     t_pagina* pagina = list_get(carpincho->paginas, numero);
     if (pagina->libre) return -1;
     memset(list_get(archivos, carpincho->archivo) + pagina->marco * tamanio_pagina, 0, tamanio_pagina);
     list_remove(carpincho->paginas, numero);
     if (asignacion == DINAMICA) {
          bitarray_clean_bit(list_get(bitmaps, carpincho->archivo), pagina->marco);
          free(pagina);
     } else {
          pagina->libre = true;
          list_add(carpincho->paginas, pagina);
     }
     return 0;
}

void limpieza() {
     log_info(logger, "Limpiando swamp...");
     for (int i = 0; i < list_size(archivos); i++) {
          memset(list_get(archivos, i), 0, tamanio_swap);
          t_bitarray* bitmap = list_get(bitmaps, i);
          for (int b = 0; b < tamanio_swap / tamanio_pagina; b++)
               bitarray_clean_bit(bitmap, b);
     }
     int cantidad_carpinchos = list_size(carpinchos);
     for (int i = 0; i < cantidad_carpinchos; i++) {
          t_carpincho* carpincho = list_get(carpinchos, 0);
          list_destroy_and_destroy_elements(carpincho->paginas, free);
          list_remove(carpinchos, 0);
          free(carpincho);
     }
}

void recibir(int cliente, int operacion) {
     t_list* valores;
     int pid, pagina;
     void* contenido;
     switch (operacion) {
          case HANDSHAKE_CARPINCHO:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               log_info(logger, "Se conecto el carpincho %d", pid);
               enviar_numero(cliente, iniciar_carpincho(pid));
               break;
          case CLOSE_CARPINCHO:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               destruir_carpincho(pid);
               log_info(logger, "Se elimino el carpincho %d", pid);
               break;
          case PAGE_READ:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               pagina = *((int*) list_get(valores, 1));
               contenido = leer_pagina(pid, pagina);
               t_paquete* paquete = crear_paquete(contenido == NULL ? MATE_READ_FAULT : 0);
               if (contenido != NULL) agregar_a_paquete(paquete, contenido, tamanio_pagina);
               enviar_paquete(paquete, cliente);
               eliminar_paquete(paquete);
               break;
          case PAGE_WRITE:
               log_info(logger, "Escribiendo pagina...");
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               pagina = *((int*) list_get(valores, 1));
               contenido = list_get(valores, 2);
               enviar_numero(cliente, escribir_pagina(pid, pagina, contenido));
               break;
          case PAGE_REMOVE:
               valores = recibir_paquete(cliente);
               pid = *((int*) list_get(valores, 0));
               pagina = *((int*) list_get(valores, 1));
               enviar_numero(cliente, eliminar_pagina(pid, pagina));
               break;
          default:
               log_info(logger, "Operacion no reconocida!");
               break;
     }
     list_destroy_and_destroy_elements(valores, free);
}

int main(void) {
     t_config* config = crear_config();
     logger = crear_logger();
     carpinchos = list_create();

     get_config(config);
     iniciar_archivos(config_get_array_value(config, "ARCHIVOS_SWAP"));

     // pruebas(); return EXIT_SUCCESS;

     int servidor = crear_servidor(config_get_int_value(config, "PUERTO"));

     config_destroy(config);

     while (1) {
          socket_memoria = esperar_cliente(servidor, logger);
          int mensaje = recibir_operacion(socket_memoria);
          if (mensaje != HANDSHAKE_MEMORIA) continue;
          enviar_numero(socket_memoria, HANDSHAKE_SWAMP);
          asignacion = recibir_numero(socket_memoria);
          log_info(logger, "Se conecto la memoria! (asignacion %s)", asignacion == FIJA ? "fija" : "dinamica");
          esperar_mensajes(argumentos(*recibir, socket_memoria, logger));
          limpieza();
     }

     return EXIT_SUCCESS;
}







void pruebas() {
     asignacion = FIJA;
     marcos_por_carpincho = 3;
     char* a = (char*) malloc(tamanio_pagina), *b = (char*) malloc(tamanio_pagina), *c = (char*) malloc(tamanio_pagina);
     for (int i = 0; i < tamanio_pagina; i++) {
          a[i] = 'a'; b[i] = 'b'; c[i] = 'c';
     }
     iniciar_carpincho(1);
     iniciar_carpincho(2);
     iniciar_carpincho(3);
     iniciar_carpincho(4);
     escribir_pagina(1, 0, b);
     escribir_pagina(2, 0, a);
     escribir_pagina(3, 0, c);
     escribir_pagina(4, 0, c);
     escribir_pagina(4, 1, c);
     char* leido = (char*) leer_pagina(1, 0);
     leido = realloc(leido, tamanio_pagina + 1);
     leido[tamanio_pagina] = '\0';
     printf("Leido: %s\n", leido);
     free(leido);
     destruir_carpincho(3);
     eliminar_pagina(4, 0);
     escribir_pagina(4, 1, c);
     destruir_carpincho(1);
     iniciar_carpincho(5);
     escribir_pagina(5, 1, c);
     dump_swap();
     free(a); free(b); free(c);
}

void dump_swap() {
     FILE* destino = stdout;
     t_list* lista_carpinchos = list_create();
     t_list* lista_marcos = list_create();
     for (int i = 0; i < list_size(archivos); i++) {
          list_add(lista_carpinchos, list_create());
          list_add(lista_marcos, list_create());
     }
     for (int i = 0; i < list_size(carpinchos); i++) {
          t_carpincho* carpincho = list_get(carpinchos, i);
          if (carpincho->archivo == -1) continue;
          t_list* carpinchos_archivo = list_get(lista_carpinchos, carpincho->archivo);
          t_list* marcos_archivo = list_get(lista_marcos, carpincho->archivo);
          for (int j = 0; j < list_size(carpincho->paginas); j++) {
               t_pagina* pagina = list_get(carpincho->paginas, j);
               if (list_is_empty(marcos_archivo)) {
                    list_add(carpinchos_archivo, carpincho);
                    list_add(marcos_archivo, pagina);
                    continue;
               }
               for (int b = 0; b < list_size(marcos_archivo); b++) {
                    t_pagina* pagina_marco = list_get(marcos_archivo, b);
                    if (pagina->marco < pagina_marco->marco) {
                         list_add_in_index(carpinchos_archivo, b, carpincho);
                         list_add_in_index(marcos_archivo, b, pagina);
                         break;
                    } else if (b == list_size(marcos_archivo) - 1) {
                         list_add(carpinchos_archivo, carpincho);
                         list_add(marcos_archivo, pagina);
                         break;
                    }
               }
          }
     }
     for (int i = 0; i < list_size(archivos); i++) {
          fprintf(destino, "Archivo %d -------------------------------------------------\n", i + 1);
          t_list* carpinchos_archivo = list_get(lista_carpinchos, i);
          t_list* marcos_archivo = list_get(lista_marcos, i);
          for (int j = 0; j < tamanio_swap / tamanio_pagina; j++) {
               if (list_is_empty(marcos_archivo)) {
                    fprintf(destino, "Marco: %d\tCarpincho: -\t\tEstado: Libre\n", j);
                    continue;
               }
               t_pagina* pagina = list_get(marcos_archivo, 0);
               if (j != pagina->marco) {
                    fprintf(destino, "Marco: %d\tCarpincho: -\t\tEstado: Libre\n", j);
               } else {
                    t_carpincho* carpincho = list_get(carpinchos_archivo, 0);
                    fprintf(destino, "Marco: %d\tCarpincho: %d\t\tEstado: %s\n", j, carpincho->pid, pagina->libre ? "Libre" : "Ocupado");
                    list_remove(marcos_archivo, 0);
                    list_remove(carpinchos_archivo, 0);
               }
          }
     }
     void destruir_lista(void* lista) { list_destroy((t_list*) lista); }
     list_destroy_and_destroy_elements(lista_carpinchos, &destruir_lista);
     list_destroy_and_destroy_elements(lista_marcos, &destruir_lista);
}