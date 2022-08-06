#include "instrucciones.h"

bool es_no_op(t_instruccion* instruccion) {
    return strcmp(instruccion->identificador, "NO_OP") == 0;
}

t_instruccion* clonar_instruccion(t_instruccion* vieja) {
    t_instruccion* nueva = malloc(sizeof(t_instruccion));
    nueva->identificador = malloc(strlen(vieja->identificador) + 1);
    memcpy(nueva->identificador, vieja->identificador, strlen(vieja->identificador) + 1);
    nueva->parametros = list_create();
    for (int i = 0; i < list_size(vieja->parametros); i++) {
        uint32_t* nuevo = malloc(sizeof(uint32_t));
        memcpy(nuevo, list_get(vieja->parametros, i), sizeof(uint32_t));
        list_add(nueva->parametros, nuevo);
    }
    return nueva;
}

void agregar_n_veces(t_list* lista, t_instruccion* instruccion, int n) {
    list_add(lista, instruccion);
    for (int i = 0; i < n - 1; i++)
        list_add(lista, clonar_instruccion(instruccion));
}

void agregar_instruccion(t_list* instrucciones, t_instruccion* instruccion) {
    int cantidad = 1;
    if (es_no_op(instruccion) && list_size(instruccion->parametros) > 0) {
        cantidad = *((uint32_t*) list_get(instruccion->parametros, 0));
        list_clean_and_destroy_elements(instruccion->parametros, free);
    } 
    agregar_n_veces(instrucciones, instruccion, cantidad);
}

char* leer_identificador(char** separado) {
    char* identificador = malloc(strlen(separado[0]) + 1);
    memcpy(identificador, separado[0], strlen(separado[0]));
    identificador[strlen(separado[0])] = '\0';
    return identificador;
}

t_list* leer_parametros(char** separado) {
    t_list* parametros = list_create();
    for (int i = 1; i < string_array_size(separado); i++) {
        uint32_t* parametro = malloc(sizeof(uint32_t));
        *parametro = (uint32_t) atoi(separado[i]);
        list_add(parametros, parametro);
    }
    return parametros;
}

t_list* deserializar_instrucciones(char* contenido_archivo) {
    t_list* instrucciones = list_create();
    char** lineas = string_split(contenido_archivo, "\n");
    for (int i = 0; i < string_array_size(lineas); i++) {
        t_instruccion* instruccion = malloc(sizeof(t_instruccion));
        // Separo el string
        char** separado = string_split(lineas[i], " ");
        // Leo el identificador
        instruccion->identificador = leer_identificador(separado);
        // Leo los parametros
        instruccion->parametros = leer_parametros(separado);
        // Agrego la instruccion a la lista (verificando si es NO_OP)
        agregar_instruccion(instrucciones, instruccion);
        // Libero memoria
        string_array_destroy(separado);
    }
    string_array_destroy(lineas);
    return instrucciones;
}

void destruir_instruccion(void* elemento) {
     t_instruccion* instruccion = (t_instruccion*) elemento;
     free(instruccion->identificador);
     list_destroy_and_destroy_elements(instruccion->parametros, free);
     free(instruccion);
}

void destruir_instrucciones(t_list* instrucciones) {
    list_destroy_and_destroy_elements(instrucciones, destruir_instruccion);
}

/* No testeado */
/* uint32_t tamanio_instruccion(t_instruccion* instruccion) { */
/*     uint32_t size = strlen(instruccion->identificador) + 1; // + 1 es el \n del final */
/*     for (int i = 0; i < list_size(instruccion->parametros); i++) */
/*         size += strlen((char*) list_get(instruccion->parametros, i)); */
/*     return size; */
/* } */

/* No testeado */
/* uint32_t tamanio_instrucciones(t_list* instrucciones) { */
/*     uint32_t size = 1; // el \0 del final */
/*     for (int i = 0; i < list_size(instrucciones); i++) */
/*         size += tamanio_instruccion((t_instruccion*) list_get(instrucciones, i)); */
/*     return size; */
/* } */

/* No testeado */
/* void* comprimir_instruccion(t_instruccion* instruccion) { */
/*     int size = tamanio_instruccion(instruccion); */
/*     void* result = malloc(size); */
/*     memcpy(result, instruccion->identificador, strlen(instruccion->identificador)); */
/*     int copied = strlen(instruccion->identificador); */
/*     for (int i = 0; i < list_size(instruccion->parametros); i++) { */
/*         char* parametro = list_get(instruccion->parametros, i); */
/*         memcpy(result + copied, parametro, strlen(parametro)); */
/*         copied += strlen(parametro); */
/*     } */
/*     return result; */
/* } */

/* No testeado */
/* char* serializar_instrucciones(t_list* instrucciones) { */
/*     int size = tamanio_instrucciones(instrucciones); */
/*     char* result = malloc(size + 1); */
/*     int copied = 0; */
/*     for (int i = 0; i < list_size(instrucciones); i++) { */
/*         char* instruccion = string_instruccion(list_get(instrucciones, i)); */
/*         memcpy(result + copied, instruccion, strlen(instruccion)); */
/*         copied += strlen(instruccion) + 1; */
/*         result[copied-1] = '\n'; */
/*         free(instruccion); */
/*     } */
/*     result[size] = '\0'; */
/*     return result; */
/* } */
