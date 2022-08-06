#include "socket.h"

void escuchar(int servidor, t_log* logger, void(*recibir)(int,int)) {
	log_info(logger, "El servidor esta escuchando...");
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

	connect(cliente, (void*) &direccion_servidor, sizeof(direccion_servidor));
	// verificar(connect(cliente, (void*) &direccion_servidor, sizeof(direccion_servidor)), "No se pudo crear la conexion al socket.");

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
	return NULL;
}

void enviar_numero(int cliente, int32_t numero)
{
	verificar( send(cliente, &numero, sizeof(int32_t), MSG_NOSIGNAL), "Error al hacer el send" );
}

void reenviar_numero(int desde, int hacia)
{
	enviar_numero(hacia, recibir_numero(desde));
}

int32_t recibir_numero(int cliente)
{
	int32_t numero;
	verificar( recv(cliente, &numero, sizeof(int32_t), MSG_WAITALL), "Error al recibir el numero" );
	return numero;
}

int recibir_operacion(int cliente) {
	int operacion;
	if(recv(cliente, &operacion, sizeof(int), MSG_WAITALL) != 0) return operacion;
	return -1;
}

void* recibir_contenido(int cliente, int size)
{
	void * buffer = malloc(size);
	verificar( recv(cliente, buffer, size, MSG_WAITALL) , "Error en el recv" );
	return buffer;
}

args* argumentos(void(*accion)(int,int), int cliente, t_log* logger){
	args* a = malloc(sizeof(args));
	a->accion = accion;
	a->cliente = cliente;
	a->logger = logger;
	return a;
}

void verificar(int resultado, char* mensaje) 
{
	if (resultado == -1) {
		printf("%s\n", mensaje);
		exit(EXIT_FAILURE);
	}
}