#include "consola.h"

#include <pcb.h>
#include <stdio.h>

char* config_path = "./config.cfg";

int main(int argc, char** argv) {
    verificar(argc == 3, "Se esperaban 2 parametros");
    // Leo las instrucciones y el tamanio
    char* instrucciones = leer_archivo(argv[2]);
    uint32_t tamanio = atoi(argv[1]);
    // Leo la config y creo la conexion a kernel
    int conexion = conectar_kernel(config_path);
    // Hago el handshake con el kernel
    handshake(conexion);
    // Envio los valores a kernel
    enviar_instrucciones(conexion, instrucciones, tamanio);
    // Espero mensaje de finalizacion
    esperar_finalizacion(conexion);
    // Me retiro de manera exitosa
    return EXIT_SUCCESS;
}
