#ifndef CONSTANTES_H_
#define CONSTANTES_H_

#define FIJA 0
#define DINAMICA 1
#define FIFO 0
#define LRU 1
#define CLOCK 2

enum mate_errors {
    MATE_FREE_FAULT = -5,
    MATE_READ_FAULT = -6,
    MATE_WRITE_FAULT = -7,
};

typedef enum {
	// General
	HANDSHAKE_CARPINCHO,
	HANDSHAKE_KERNEL,
	HANDSHAKE_MEMORIA,
	HANDSHAKE_SWAMP,
	CLOSE_CARPINCHO,
	// Kernel
	SEM_INIT,
	SEM_WAIT,
	SEM_POST,
	SEM_DESTROY,
	CALL_IO,
	// Memoria
	MEM_MALLOC,
	MEM_FREE,
	MEM_READ,
	MEM_WRITE,
	// Swamp
	PAGE_WRITE,
	PAGE_READ,
	PAGE_REMOVE
} op_code;

typedef enum {
	NEW,
	READY,
	READY_SUSPEND,
	EXEC,
	BLOCK,
	BLOCK_SUSPEND,
	EXIT
} t_estado;

typedef enum {
	SJF,
	HRRN
} t_algoritmo;

#endif