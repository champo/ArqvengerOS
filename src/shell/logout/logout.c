#include "shell/logout/logout.h"
#include "mcurses/mcurses.h"
#include "library/stdio.h"
#include "library/sys.h"



void logout(char* argv) {
    exit();
}


void manLogout(void) {
    setBold(1);
    printf("Usage:\n\t logout\n");
    setBold(0);
}
