#include "archivo.h"

char* leer_archivo(char* path){
     FILE *f = fopen(path, "r");
     verificar(f != NULL, "No se pudo abrir el archivo");
     fseek(f, 0, SEEK_END);
     long fsize = ftell(f);
     fseek(f, 0, SEEK_SET);
     char *string = malloc(fsize + 1);
     fread(string, fsize, 1, f);
     fclose(f);
     string[fsize] = '\0';
     return string;
}