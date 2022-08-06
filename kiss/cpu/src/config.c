#include "config.h"

void leer_config(char *path) {
    t_config *config = config_create(path);
    verificar(config != NULL, "Error al leer el archivo de configuracion");
    cpu_config = malloc(sizeof(t_cpu_config));
    cpu_config->entradas_tlb = config_get_int_value(config, "ENTRADAS_TLB");
    char *algoritmo = config_get_string_value(config, "REEMPLAZO_TLB");
    cpu_config->algoritmo_reemplazo_tlb =
        strcmp(algoritmo, "FIFO") == 0  ? FIFO :
        strcmp(algoritmo, "LRU") == 0 ? LRU : -1;
    cpu_config->retardo_noop = config_get_int_value(config, "RETARDO_NOOP");
    // ip memoria
    char* ip_memoria = getenv("IP_MEMORIA");
    if (ip_memoria == NULL) ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    cpu_config->ip_memoria = malloc(strlen(ip_memoria) + 1);
    memcpy(cpu_config->ip_memoria, ip_memoria, strlen(ip_memoria));
    cpu_config->ip_memoria[strlen(ip_memoria)] = '\0';
    // puerto memoria
    const char* puerto_memoria = getenv("PUERTO_MEMORIA");
    cpu_config->puerto_memoria = puerto_memoria != NULL ? atoi(puerto_memoria)
        : config_get_int_value(config, "PUERTO_MEMORIA");
    // puertos cpu
    const char* puerto_dispatch = getenv("PUERTO_ESCUCHA_DISPATCH");
    cpu_config->puerto_cpu_dispatch = puerto_dispatch != NULL ? atoi(puerto_dispatch)
        : config_get_int_value(config, "PUERTO_ESCUCHA_DISPATCH");
    const char* puerto_interrupt = getenv("PUERTO_ESCUCHA_INTERRUPT");
    cpu_config->puerto_cpu_interrupt = puerto_interrupt != NULL ? atoi(puerto_interrupt)
        : config_get_int_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    config_destroy(config);
    printf("MEMORIA=%s:%d CPU=%d(d):%d(i)\n", cpu_config->ip_memoria, cpu_config->puerto_memoria, cpu_config->puerto_cpu_dispatch, cpu_config->puerto_cpu_interrupt);
}
