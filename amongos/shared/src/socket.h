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

#include "utils.h"

typedef struct {
	int cliente; 
	t_log* logger; 
	void(*accion)(int,int);
} args;

args* argumentos(void(*accion)(int,int), int cliente, t_log* logger);
void escuchar(int servidor, t_log* logger, void(*recibir)(int,int));
int crear_servidor(unsigned puerto);
int esperar_cliente(int servidor, t_log* logger);
int recibir_operacion(int cliente);
void* esperar_mensajes(void* args);
int crear_conexion(char* ip, int puerto);

#endif
