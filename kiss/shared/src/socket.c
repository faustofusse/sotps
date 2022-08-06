#include "socket.h"

void* _recibir_conexiones(void* a) {
    int servidor = ((args*) a)->conexion;
    t_list* handlers = ((args*) a)->handlers;
    t_log* logger = ((args*) a)->logger;
	log_info(logger, "El servidor esta escuchando...");
	while (1) {
		int cliente = recibir_conexion(servidor, logger);
		pthread_t hilo;
		pthread_create(&hilo, NULL, *recibir_mensajes, (void*) argumentos(cliente, handlers, logger));
		pthread_detach(hilo);
    }
    return NULL;
}

void recibir_conexiones(int servidor, t_list* handlers, t_log* logger, bool en_otro_hilo) {
    args* a = argumentos(servidor, handlers, logger);
    if (en_otro_hilo) {
        pthread_t hilo;
		pthread_create(&hilo, NULL, *_recibir_conexiones, a);
		pthread_detach(hilo);
    } else _recibir_conexiones(a);
}

int crear_conexion(char* ip, int puerto) {
	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(ip);
	direccion_servidor.sin_port = htons(puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	verificar(
		connect(cliente, (void*) &direccion_servidor, sizeof(direccion_servidor)) == 0,
		"No se pudo crear la conexion al socket."
	);

	return cliente;
}

int crear_servidor(unsigned puerto) {
	int servidor, verdadero = 1;

	struct sockaddr_in direccion_servidor;
	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;
	direccion_servidor.sin_port = htons(puerto);

	verificar((servidor = socket(AF_INET, SOCK_STREAM, 0)), "Error al crear el socket.");

	verificar(setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &verdadero, sizeof(verdadero)) == 0, "Fallo en el setsockopt.");

	verificar(bind(servidor, (void*) &direccion_servidor, sizeof(direccion_servidor)) == 0, "Fallo en el bind del soocket.");

	verificar(listen(servidor, SOMAXCONN) == 0, "Error del socket al escuchar.");

	return servidor;
}

int recibir_conexion(int servidor, t_log* logger) {
	struct sockaddr_in direccion_cliente;
	unsigned tam_direccion = sizeof(struct sockaddr_in);
	int cliente = accept(servidor, (void*) &direccion_cliente, &tam_direccion);
	log_info(logger, "Se conecto un cliente.");
	return cliente;
}

void* recibir_mensajes(void* a) {
    int cliente = ((args*) a)->conexion;
    t_list* handlers = ((args*) a)->handlers;
	t_log* logger = ((args*) a)->logger;
	free(a);
	uint32_t operacion = recibir_numero(cliente);
	while (operacion != CLOSE_CONNECTION) {
		for (int i = 0; i < list_size(handlers); i++) {
			if (operacion == ((t_handler*) list_get(handlers, i))->operacion) {
				((t_handler*) list_get(handlers, i))->funcion(cliente, logger);
				break;
			}
		}
		operacion = recibir_numero(cliente);
	}
	log_info(logger, "Se desconecto un cliente");
	return NULL;
}

void enviar_numero(int cliente, uint32_t numero) {
	verificar( send(cliente, &numero, sizeof(uint32_t), MSG_NOSIGNAL) != -1, "Error al hacer el send" );
}

uint32_t recibir_numero(int cliente) {
	uint32_t numero;
	verificar( recv(cliente, &numero, sizeof(uint32_t), MSG_WAITALL) != -1, "Error al recibir el numero" );
	return numero;
}

void* recibir_contenido(int cliente, int size) {
	void * buffer = malloc(size);
	verificar( recv(cliente, buffer, size, MSG_WAITALL) != -1, "Error en el recv" );
	return buffer;
}

args* argumentos(int conexion, t_list* handlers, t_log* logger){
	args* resultado = malloc(sizeof(args));
	resultado->conexion = conexion;
	resultado->handlers = handlers;
	resultado->logger = logger;
	return resultado;
}

void agregar_handler(t_list* handlers, op_code operacion, t_handler_function funcion) {
     t_handler* handler = malloc(sizeof(t_handler));
     handler->operacion = operacion;
     handler->funcion = funcion;
     list_add(handlers, handler);
}
