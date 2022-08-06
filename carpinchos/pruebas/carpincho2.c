#include <stdio.h>
#include <stdlib.h>
#include <matelib.h>

int main (void) {
     mate_instance carpincho1, carpincho2;
     mate_init(&carpincho1, "./config.cfg");
     mate_init(&carpincho2, "./config.cfg");
     sleep(3);
     mate_close(&carpincho1);
     mate_close(&carpincho2);
     return 0;
}