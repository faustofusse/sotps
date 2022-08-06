#include "utils.h"

void verificar(int condicion, char* mensaje) {
     if (!condicion || condicion == -1) {
          printf("%s\n", mensaje);
          exit(EXIT_FAILURE);
     }
}
