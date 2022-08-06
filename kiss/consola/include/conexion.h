#ifndef CONEXION_H_
#define CONEXION_H_

#include <stdlib.h>
#include <stdint.h>
#include <socket.h>
#include <commons/config.h>

#include "paquete.h"
#include "constantes.h"

void enviar_instrucciones(int conexion, char* instrucciones, uint32_t tamanio);
void handshake(int conexion);
int conectar_kernel(char* config_path);
void esperar_finalizacion(int conexion);

#endif
