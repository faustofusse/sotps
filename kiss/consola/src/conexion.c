#include "conexion.h"

int conectar_kernel(char* config_path) {
     t_config* config = config_create(config_path);
    char* ip_kernel = getenv("IP_KERNEL");
    char* puerto_kernel = getenv("PUERTO_KERNEL");
     int conexion = crear_conexion(
          ip_kernel == NULL ? config_get_string_value(config, "IP_KERNEL") : ip_kernel,
          puerto_kernel == NULL ? config_get_int_value(config, "PUERTO_KERNEL") : atoi(puerto_kernel)
     );
     config_destroy(config);
     return conexion;
}

void enviar_instrucciones(int conexion, char* instrucciones, uint32_t tamanio) {
     t_paquete* paquete = crear_paquete(INSTRUCCIONES);
     agregar_a_paquete(paquete, instrucciones, strlen(instrucciones) + 1);
     agregar_a_paquete(paquete, &tamanio, sizeof(uint32_t));
     enviar_paquete(paquete, conexion);
     eliminar_paquete(paquete);
     free(instrucciones);
}

void esperar_finalizacion(int conexion) {
     uint32_t numero = recibir_numero(conexion);
     if (numero != FINALIZACION_CONSOLA)
          printf("ATENCION: Se esperaba finalizacion, pero se recibio otra cosa\n");
}

void handshake(int conexion) {
     enviar_numero(conexion, HANDSHAKE_CONSOLA);
     uint32_t numero = recibir_numero(conexion);
     verificar(numero == HANDSHAKE_KERNEL, "El kernel no responde correctamente");
}
