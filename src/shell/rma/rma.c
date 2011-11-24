#include "shell/rma/rma.h"
#include "library/stdio.h"
#include "mcurses/mcurses.h"
#include "library/stdlib.h"
#include "library/time.h"

void rma(char* argv) {
    
    srand(time(NULL));
    int* address = (int*) rand();
    int b = *address;
    rma(argv);
    return;
}

void man_rma(void) {
    setBold(1);
    printf("Usage:\n\t rma");
    setBold(0);
}
