#include <stdio.h>
#include <stdlib.h>
#include <matelib.h>

int main (void) {
     int hola = 4444;
     mate_instance mate_ref;
     mate_init(&mate_ref, "./config.cfg");
     // sleep(1);
     // mate_call_io(&mate_ref, "IMPRESORA", (void*) "a;sfdjnalskdnfjsadjn");
     // sleep(1);
     // mate_sem_wait(&mate_ref, "SEMAFORO 1");
     sleep(2);
     // int direccion = mate_memalloc(&mate_ref, hola);
     // printf("Malloc devolvio la direccion %d\n", direccion);
     // sleep(1);
     // mate_memwrite(&mate_ref, &hola, hola, sizeof(int));
     // sleep(5);
     mate_close(&mate_ref);
     return 0;
}