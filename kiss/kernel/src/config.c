#include "config.h"
#include <stdlib.h>
#include <string.h>

void leer_config(char* path) {
    t_config* config = config_create(path);
    verificar(config != NULL, "Error al leer el archivo de configuracion");
    kernel_config = malloc(sizeof(t_kernel_config));
    // ip memoria
    char* ip_memoria = getenv("IP_MEMORIA");
    if (ip_memoria == NULL) ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    kernel_config->ip_memoria = malloc(strlen(ip_memoria) + 1);
    memcpy(kernel_config->ip_memoria, ip_memoria, strlen(ip_memoria));
    kernel_config->ip_memoria[strlen(ip_memoria)] = '\0';
    // puerto memoria
    const char* puerto_memoria = getenv("PUERTO_MEMORIA");
    kernel_config->puerto_memoria = puerto_memoria == NULL ? config_get_int_value(config, "PUERTO_MEMORIA")
        : atoi(puerto_memoria);
    // ip cpu
    char* ip_cpu = getenv("IP_CPU");
    if (ip_cpu == NULL) ip_cpu = config_get_string_value(config, "IP_CPU");
    kernel_config->ip_cpu = malloc(strlen(ip_cpu) + 1);
    memcpy(kernel_config->ip_cpu, ip_cpu, strlen(ip_cpu));
    kernel_config->ip_cpu[strlen(ip_cpu)] = '\0';
    // puertos cpu
    const char* puerto_cpu_dispatch = getenv("PUERTO_CPU_DISPATCH");
    kernel_config->puerto_cpu_dispatch = puerto_cpu_dispatch == NULL ? config_get_int_value(config, "PUERTO_CPU_DISPATCH")
        : atoi(puerto_cpu_dispatch);
    const char* puerto_cpu_interrupt = getenv("PUERTO_CPU_INTERRUPT");
    kernel_config->puerto_cpu_interrupt = puerto_cpu_interrupt == NULL ? config_get_int_value(config, "PUERTO_CPU_INTERRUPT")
        : atoi(puerto_cpu_interrupt);
    // puerto escucha
    const char* puerto_escucha = getenv("PUERTO_ESCUCHA");
    kernel_config->puerto_escucha = puerto_escucha == NULL ? config_get_int_value(config, "PUERTO_ESCUCHA")
        : atoi(puerto_escucha);
    // algoritmo
    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    kernel_config->algoritmo_planificacion = 
        strcmp(algoritmo, "FIFO") == 0 ? FIFO :
        strcmp(algoritmo, "SRT") == 0 ? SRT : -1;
    // etcetera
    kernel_config->estimacion_inicial = config_get_int_value(config, "ESTIMACION_INICIAL");
    kernel_config->alfa = (float) config_get_double_value(config, "ALFA");
    kernel_config->grado_multiprogramacion = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    kernel_config->tiempo_maximo_bloqueo = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
    config_destroy(config);
    printf("CPU=%s:d%d:i%d MEM=%s:%d KERNEL=%d\n", kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch, kernel_config->puerto_cpu_interrupt, kernel_config->ip_memoria, kernel_config->puerto_memoria, kernel_config->puerto_escucha);
}
