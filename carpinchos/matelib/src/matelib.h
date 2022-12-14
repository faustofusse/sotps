#ifndef MATELIB_H_
#define MATELIB_H_

#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <commons/config.h>

#include "socket.h"
#include "paquete.h"
#include "constantes.h"

//-------------------Type Definitions----------------------/

typedef struct mate_instance {
    int id;
    int servidor; // backend (ya sea kernel o memoria)
    bool kernel; // true si esta conectado al kernel, false a la memoria
} mate_instance;

typedef char *mate_io_resource;

typedef char *mate_sem_name;

typedef int32_t mate_pointer;

//------------------General Functions---------------------/

int mate_init(mate_instance *lib_ref, char *config);

int mate_close(mate_instance *lib_ref);

//-----------------Semaphore Functions---------------------/

int mate_sem_init(mate_instance *lib_ref, mate_sem_name sem, unsigned int value);

int mate_sem_wait(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_post(mate_instance *lib_ref, mate_sem_name sem);

int mate_sem_destroy(mate_instance *lib_ref, mate_sem_name sem);

//--------------------IO Functions------------------------/

int mate_call_io(mate_instance *lib_ref, mate_io_resource io, void *msg);

//--------------Memory Module Functions-------------------/

mate_pointer mate_memalloc(mate_instance *lib_ref, int size);

int mate_memfree(mate_instance *lib_ref, mate_pointer addr);

int mate_memread(mate_instance *lib_ref, mate_pointer origin, void *dest, int size);

int mate_memwrite(mate_instance *lib_ref, void *origin, mate_pointer dest, int size);

//----------------------- Other --------------------------/

int error_no_autorizado();
void enviar_string(int cliente, op_code operacion, char* string);

#endif