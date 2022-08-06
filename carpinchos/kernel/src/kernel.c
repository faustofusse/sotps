#include "kernel.h"

int main(void) {
     logger = crear_logger();
     config = crear_config();

     obtener_valores_config(config);

     socket_memoria = conectar_memoria(config);

     int servidor = crear_servidor(config_get_int_value(config, "PUERTO"));
     escuchar(servidor, logger, *recibir);

     return EXIT_SUCCESS;
}

void recibir(int cliente, int operacion){
     t_list* valores = NULL;
     char* nombre;
     int valor;
     switch (operacion) {
          case SEM_INIT:
               valores = recibir_paquete(cliente);
               nombre = list_get(valores, 0);
               valor = *((int*) list_get(valores, 1));
               printf("SEM_INIT el semaforo %s en %d\n", nombre, valor);
               break;
          case SEM_WAIT:
               valores = recibir_paquete(cliente);
               nombre = list_get(valores, 0);
               printf("SEM_WAIT el semaforo %s\n", nombre);
               break;
          case SEM_POST:
               valores = recibir_paquete(cliente);
               nombre = list_get(valores, 0);
               printf("SEM_POST el semaforo %s\n", nombre);
               break;
          case SEM_DESTROY:
               valores = recibir_paquete(cliente);
               nombre = list_get(valores, 0);
               printf("SEM_DESTROY el semaforo %s\n", nombre);
               break;
          case CALL_IO:
               valores = recibir_paquete(cliente);
               nombre = list_get(valores, 0);
               printf("CALL_IO al io de nombre %s\n", nombre);
               break;
          case MEM_MALLOC:
               reenviar_paquete(cliente, socket_memoria, operacion);
               reenviar_numero(socket_memoria, cliente);
               break;
          case MEM_READ:
               reenviar_paquete(cliente, socket_memoria, operacion);
               reenviar_paquete(socket_memoria, cliente, operacion);
               break;
          case MEM_FREE:
          case MEM_WRITE:
               reenviar_paquete(cliente, socket_memoria, operacion);
               break;
          case CLOSE_CARPINCHO:
               printf("Close carpincho!\n");
               enviar_numero(socket_memoria, CLOSE_CARPINCHO);
               break;
          case HANDSHAKE_CARPINCHO:
               log_info(logger, "Se conecto un carpincho!");
               enviar_numero(cliente, HANDSHAKE_KERNEL);
               break;
          default:
               log_info(logger, "Operacion no reconocida!");
               break;
     }
     if (valores != NULL) list_destroy_and_destroy_elements(valores, free);
}

t_config* crear_config() {
     return config_create("./config.cfg");
}

t_log* crear_logger() {
     return log_create("./kernel.log", "Kernel", 1, LOG_LEVEL_INFO);
}

int conectar_memoria(t_config* config) {
     // Creo la conexion
     int memoria = crear_conexion(
          config_get_string_value(config, "IP_MEMORIA"),
          config_get_int_value(config, "PUERTO_MEMORIA")
     );
     // Hago el handshake
     enviar_numero(memoria, HANDSHAKE_KERNEL);
     // Recibo la respuesta de la memoria
     int respuesta = recibir_operacion(memoria);
     if (respuesta == HANDSHAKE_MEMORIA) {
          log_info(logger, "Se conecto la memoria!");
     } else {
          log_info(logger, "No se pudo conectar la memoria!");
          exit(EXIT_FAILURE);
     }
     // Devuelvo el cliente
     return memoria;
}

void obtener_valores_config(t_config* c) {
     // Constantes
     algoritmo_planificacion = config_get_int_value(c, "ALGORITMO_PLANIFICACION");
     estimacion_inicial = config_get_int_value(c, "ESTIMACION_INICIAL");
     retardo_cpu = config_get_int_value(c, "RETARDO_CPU");
     multiprogramacion = config_get_int_value(c, "GRADO_MULTIPROGRAMACION");
     multiprocesamiento = config_get_int_value(c, "GRADO_MULTIPROCESAMIENTO");
     tiempo_deadlock = config_get_int_value(c, "TIEMPO_DEADLOCK");
     alfa = config_get_double_value(c, "ALFA");
     // Dispositivos IO
     char** nombres = string_get_string_as_array(config_get_string_value(c, "DISPOSITIVOS_IO"));
     char** duraciones = string_get_string_as_array(config_get_string_value(c, "DURACIONES_IO"));
     while (*nombres != NULL) {
          t_io* io = malloc(sizeof(t_io));
          io->nombre = *nombres;
          io->duracion = atoi(*duraciones);
          list_add(dispositivos_io, io);
          nombres++;
          duraciones++;
     }
     for (int i = 0; i < list_size(dispositivos_io); i++) {
          t_io* io = list_get(dispositivos_io, i);
          log_info(logger, "IO %s con duracion %d", io->nombre, io->duracion);
     }
}