#include "shell/kill/kill.h"
#include "library/stdlib.h"
#include "library/stdio.h"
#include "library/sys.h"

static void error(void) {
    printf("Usage: kill pid\n");
    exit();
}

void killCmd(char* args) {

    char* firstSpace = strchr(args, ' ');
    if (firstSpace == NULL) {
        error();
    }

    pid_t pid = atoi(firstSpace);
    if (pid == 0) {
        return;
        error();
    }

    kill(pid);
}

void manKill(void) {
    setBold(1);
    printf("Usage:\n\tkill");
    setBold(0);

    printf(" pid\n\n");
}

