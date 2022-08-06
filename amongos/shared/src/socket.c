#include "socket.h"

void escuchar(int servidor, t_log* logger, void(*recibir)(int,int)) {
	while (1) {
		int cliente = esperar_cliente(servidor, logger);
		pthread_t hilo;
		pthread_create(&hilo, NULL, *esperar_mensajes, (void*) argumentos(recibir, cliente, logger));
		pthread_detach(hilo);
	}
}

int crear_conexion(char* ip, int puerto) {
	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(ip);
	direccion_servidor.sin_port = htons(puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	verificar(connect(cliente, (void*) &direccion_servidor, sizeof(direccion_servidor)), "No se pudo crear la conexion al socket.");

	return cliente;
}

int crear_servidor(unsigned puerto) {
	int servidor, verdadero = 1;

	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;
	direccion_servidor.sin_port = htons(puerto);

	verificar((servidor = socket(AF_INET, SOCK_STREAM, 0)), "Error al crear el socket.");

	verificar(setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &verdadero, sizeof(verdadero)), "Fallo en el setsockopt.");

	verificar(bind(servidor, (void*) &direccion_servidor, sizeof(direccion_servidor)), "Fallo en el bind del soocket.");

	verificar(listen(servidor, SOMAXCONN), "Error del socket al escuchar.");

	return servidor;
}

int esperar_cliente(int servidor, t_log* logger) {
	struct sockaddr_in direccion_cliente;
	unsigned tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept(servidor, (void*) &direccion_cliente, &tam_direccion);

	log_info(logger, "Se conecto un cliente!");

	return cliente;
}

void* esperar_mensajes(void* argumentos) {
	int cliente = ((args*)argumentos)->cliente;
	t_log* logger = ((args*)argumentos)->logger;
	void(*accion)(int,int) = ((args*)argumentos)->accion;
	free(argumentos);
	int operacion = recibir_operacion(cliente);
	while (operacion != -1) {
		(accion)(cliente, operacion);
		operacion = recibir_operacion(cliente);
	}
	log_info(logger, "Se desconecto un cliente.");
	return NULL;
}

int recibir_operacion(int cliente) {
	int operacion;
	if(recv(cliente, &operacion, sizeof(int), MSG_WAITALL) != 0) return operacion;
	return -1;
}

args* argumentos(void(*accion)(int,int), int cliente, t_log* logger){
	args* a = malloc(sizeof(args));
	a->accion = accion;
	a->cliente = cliente;
	a->logger = logger;
	return a;
}