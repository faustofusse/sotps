#include "matelib.h"

//------------------General Functions---------------------/

int mate_init(mate_instance *lib_ref, char *config) {
     // Abro el archivo de configuracion
     t_config *cfg = config_create(config);
     if (!cfg) {
          printf("Error: no se pudo abrir el archivo de configuracion %s\n", config);
          return -1;
     }
     // Me conecto al servidor
     int servidor = crear_conexion(
          config_get_string_value(cfg, "IP_BACKEND"),
          config_get_int_value(cfg, "PUERTO_BACKEND")
     );
     if (servidor == -1) {
          printf("Error: no se pudo conectar con el servidor\n");
          return -1;
     }
     lib_ref->servidor = servidor;
     // Hago handshake
     enviar_numero(servidor, HANDSHAKE_CARPINCHO);
     int handshake = recibir_operacion(servidor);
     if (handshake != HANDSHAKE_KERNEL && handshake != HANDSHAKE_MEMORIA) {
          printf("Error: no se pudo conectar con el servidor\n");
          return -1;
     }
     lib_ref->kernel = handshake == HANDSHAKE_KERNEL;
     // Recibo el ID de la referencia
     lib_ref->id = recibir_numero(servidor);
     // Destruyo el archivo de configuracion
     config_destroy(cfg);
     // printeo
     printf("Se inicio el carpincho correctamente (id: %d)\n", lib_ref->id);
     return 0;
}

int mate_close(mate_instance *lib_ref) {
     t_paquete* paquete = crear_paquete(CLOSE_CARPINCHO);
     agregar_a_paquete(paquete, &(lib_ref->id), sizeof(int));
     enviar_paquete(paquete, lib_ref->servidor);
     eliminar_paquete(paquete);
     close(lib_ref->servidor);
     return 0;
}

//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value) {
     if (!lib_ref->kernel) return error_no_autorizado();
     t_paquete *paquete = crear_paquete(SEM_INIT);
     agregar_a_paquete(paquete, sem, strlen(sem) + 1);
     agregar_a_paquete(paquete, &value, sizeof(unsigned int));
     enviar_paquete(paquete, lib_ref->servidor);
     eliminar_paquete(paquete);
     return 0;
}

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem) {
     if (!lib_ref->kernel) return error_no_autorizado();
     enviar_string(lib_ref->servidor, SEM_WAIT, sem);
     return 0;
}

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem) {
     if (!lib_ref->kernel) return error_no_autorizado();
     enviar_string(lib_ref->servidor, SEM_POST, sem);
     return 0;
}

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem) {
     if (!lib_ref->kernel) return error_no_autorizado();
     enviar_string(lib_ref->servidor, SEM_DESTROY, sem);
     return 0;
}

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg) {
     if (!lib_ref->kernel) return error_no_autorizado();
     enviar_string(lib_ref->servidor, CALL_IO, io);
     return 0;
}

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size) {
     // Pido la reserva de memoria
     t_paquete *paquete = crear_paquete(MEM_MALLOC);
     agregar_a_paquete(paquete, &lib_ref->id, sizeof(int));
     agregar_a_paquete(paquete, &size, sizeof(int));
     enviar_paquete(paquete, lib_ref->servidor);
     eliminar_paquete(paquete);
     // Recibo la direccion de memoria
     return recibir_numero(lib_ref->servidor);
}

int mate_memfree(mate_instance *lib_ref, mate_pointer addr) {
     t_paquete *paquete = crear_paquete(MEM_FREE);
     agregar_a_paquete(paquete, &addr, sizeof(mate_pointer));
     enviar_paquete(paquete, lib_ref->servidor);
     eliminar_paquete(paquete);
     return 0;
}

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size) {
     // Pido la lectura
     t_paquete *paquete = crear_paquete(MEM_READ);
     agregar_a_paquete(paquete, &origin, sizeof(mate_pointer));
     agregar_a_paquete(paquete, &size, sizeof(int));
     enviar_paquete(paquete, lib_ref->servidor);
     eliminar_paquete(paquete);
     // Recibo el contenido
     if (recibir_operacion(lib_ref->servidor) != MEM_READ) {
          printf("Error: no se pudo leer la memoria\n");
          return -1;
     }
     t_list* valores = recibir_paquete(lib_ref->servidor);
     void* contenido = list_get(valores, 0);
     free(contenido);
     list_destroy(valores);
     return 0;
}

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size) {
     t_paquete *paquete = crear_paquete(MEM_WRITE);
     agregar_a_paquete(paquete, &dest, sizeof(mate_pointer));
     agregar_a_paquete(paquete, &size, sizeof(int));
     agregar_a_paquete(paquete, origin, size);
     enviar_paquete(paquete, lib_ref->servidor);
     eliminar_paquete(paquete);
     return 0;
}

//----------------------- Other --------------------------/

int error_no_autorizado() {
     printf("No se pueden utilizar las funciones de kernel!\n");
     return -1;
}

void enviar_string(int cliente, op_code operacion, char* string) {
     t_paquete *paquete = crear_paquete(operacion);
     agregar_a_paquete(paquete, string, strlen(string) + 1);
     enviar_paquete(paquete, cliente);
     eliminar_paquete(paquete);
}
