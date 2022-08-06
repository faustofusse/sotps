#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "constantes.h"

typedef struct {
	int cliente; 
	t_log* logger; 
	void(*accion)(int,int);
} args;

void escuchar(int servidor, t_log* logger, void(*recibir)(int,int));
void* esperar_mensajes(void* args);
int crear_servidor(unsigned puerto);
int crear_conexion(char* ip, int puerto);
int esperar_cliente(int servidor, t_log* logger);

void* recibir_contenido(int cliente, int size);
void enviar_numero(int cliente, int32_t numero);
int32_t recibir_numero(int cliente);
void reenviar_numero(int desde, int hacia);
int recibir_operacion(int cliente);

void verificar(int resultado, char* mensaje);
args* argumentos(void(*accion)(int,int), int cliente, t_log* logger);

#endif
