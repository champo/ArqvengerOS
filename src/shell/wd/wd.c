#include "shell/wd/wd.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "mcurses/mcurses.h"

void cd(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("cd needs at least one parameter\n");
        return;
    }

    cmdEnd++;
    if (chdir(cmdEnd) == -1) {
        printf("%s is not a directory.\n", cmdEnd);
    }
}

void manCd(void) {
    setBold(1);
    printf("Usage:\n\t cd");
    setBold(0);

    printf(" directory\n");
}

void pwd(char* argv) {
    char cwd[512];
    if (getcwd(cwd, 512) == 0) {
        printf("%s\n", cwd);
    } else {
        printf("Oops. Something went horribly wrong! Please do tell Champo about it.");
    }
}

void manPwd(void) {
    setBold(1);
    printf("Usage:\n\t pwd");
    setBold(0);
}

