#include "mapa.h"

typedef struct {
     int pid;
     int tid;
     char caracter;
} tripulante;

t_list* ids;
NIVEL* nivel;

pthread_mutex_t mutex_ids;
pthread_mutex_t mutex_mapa;

void mapa_iniciar()
{
     nivel_gui_inicializar();
     char* titulo = string_from_format("Mapa - PID: %d", getpid());
     nivel = nivel_crear(titulo);
     free(titulo);
     ids = list_create();
     pthread_mutex_init(&mutex_ids, NULL);
     pthread_mutex_init(&mutex_mapa, NULL);
     nivel_gui_dibujar(nivel);
}

char get_id(int pid, int tid)
{
     pthread_mutex_lock(&mutex_ids);
     for (int i = 0; i < list_size(ids); i++) {
          tripulante* t = (tripulante*) list_get(ids, i);
          if (t->pid == pid && t->tid == tid) {
               pthread_mutex_unlock(&mutex_ids);
               return t->caracter;
          }
     }
     pthread_mutex_unlock(&mutex_ids);
     return '\0'; // ????
}

void mapa_agregar_tripulante(int pid, int tid, int x, int y, char caracter)
{
     tripulante* t = malloc(sizeof(tripulante));
     t->pid = pid;
     t->tid = tid;
     t->caracter = caracter;
     pthread_mutex_lock(&mutex_ids);
     list_add(ids, t);
     pthread_mutex_unlock(&mutex_ids);
     pthread_mutex_lock(&mutex_mapa);
     personaje_crear(nivel, caracter, x, y);
     nivel_gui_dibujar(nivel);
     pthread_mutex_unlock(&mutex_mapa);
}

void mapa_sacar_tripulante(int pid, int tid)
{
     char id;
     pthread_mutex_lock(&mutex_ids);
     for (int i = 0; i < list_size(ids); i++) {
          tripulante* t = (tripulante*) list_get(ids, i);
          if (t->pid == pid && t->tid == tid) {
               id = t->caracter;
               list_remove(ids, i);
               free(t);
               break;
          }
     }
     pthread_mutex_unlock(&mutex_ids);
     pthread_mutex_lock(&mutex_mapa);
     item_borrar(nivel, id);
     nivel_gui_dibujar(nivel);
     pthread_mutex_unlock(&mutex_mapa);
}

void mapa_mover_tripulante(int pid, int tid, int x, int y)
{
     pthread_mutex_lock(&mutex_mapa);
     item_mover(nivel, get_id(pid, tid), x, y);
     nivel_gui_dibujar(nivel);
     pthread_mutex_unlock(&mutex_mapa);
}

void mapa_cambiar_estado(int pid, int tid, char estado)
{

}