#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "i-Mongo-Store.h"

char buffer[100];


typedef struct {
	u_int32_t pid;
	u_int32_t tareas;
} PCB;


typedef struct {
	u_int32_t tid;
	char estado;
	u_int32_t x;
	u_int32_t y;
	u_int32_t instruccion;
	u_int32_t PCB;
} TCB;

typedef struct {
	uint32_t block_size;
	uint32_t blocks;
	bool bitmap;
} SuperBloque;

bool existe_filesystem();

#endif
