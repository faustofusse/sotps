#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "constantes.h"
#include "utils.h"

typedef struct {
    int conexion;
    t_list* handlers;
    t_log* logger;
} args;

typedef void (*t_handler_function)(int conexion, t_log* logger);

typedef struct {
     op_code operacion;
     t_handler_function funcion;
} t_handler;

int crear_servidor(unsigned puerto);
int crear_conexion(char* ip, int puerto);
int recibir_conexion(int servidor, t_log* logger);
void recibir_conexiones(int servidor, t_list* handlers, t_log* logger, bool en_otro_hilo);
void* recibir_mensajes(void* args);

void* recibir_contenido(int cliente, int size);
void enviar_numero(int cliente, uint32_t numero);
uint32_t recibir_numero(int cliente);
void agregar_handler(t_list* handlers, op_code operacion, t_handler_function funcion);

void verificar(int resultado, char* mensaje);
args* argumentos(int conexion, t_list* handlers, t_log* logger);

#endif
