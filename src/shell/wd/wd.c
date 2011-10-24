#include "shell/wd/wd.h"
#include "library/stdio.h"
#include "constants.h"
#include "library/stdlib.h"
#include "library/string.h"
#include "mcurses/mcurses.h"

void command_cd(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("Usage: cd path\n");
        return;
    }

    cmdEnd++;
    if (chdir(cmdEnd) == -1) {
        printf("%s is not a directory.\n", cmdEnd);
    }
}

void man_cd(void) {
    setBold(1);
    printf("Usage:\n\t cd");
    setBold(0);

    printf(" directory\n");
}

void command_pwd(char* argv) {
    char cwd[512];
    if (getcwd(cwd, 512) == 0) {
        printf("%s\n", cwd);
    } else {
        printf("Oops. Something went horribly wrong! Please do tell Champo about it.");
    }
}

void man_pwd(void) {
    setBold(1);
    printf("Usage:\n\t pwd");
    setBold(0);
}

void command_mkdir(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("Usage: mkdir path\n");
        return;
    }

    cmdEnd++;
    switch (mkdir(cmdEnd, 0744)) {
        case -1:
            printf("Cant create '%s': No such file or directory.\n", cmdEnd);
            break;
        case EEXIST:
            printf("Cant create '%s': File already exists.\n", cmdEnd);
            break;
        case EIO:
            printf("Cant create '%s': Some wizardry went awry. Try again later.\n", cmdEnd);
            break;
    }
}

void command_rmdir(char* argv) {

    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        printf("Usage: rmdir path\n");
        return;
    }

    cmdEnd++;
    switch (rmdir(cmdEnd)) {
        case -1:
        case ENOENT:
            printf("Cant remove '%s': No such file or directory.\n", cmdEnd);
            break;
        case EIO:
            printf("Cant remove '%s': Some wizardry went awry. Try again later.\n", cmdEnd);
            break;
        case ENOTEMPTY:
            printf("Cant remove '%s': The directory is not empty.\n", cmdEnd);
            break;
    }
}

void command_ls(char* argv) {

    int fd;
    char* cmdEnd = strchr(argv, ' ');
    if (cmdEnd == NULL) {
        fd = open(".", O_RDONLY);
    } else {
        fd = open(cmdEnd + 1, O_RDONLY);
    }

    struct fs_DirectoryEntry entry;

    while (readdir(fd, &entry) == 1) {
        if (entry.name[0] != '.') {
            printf("%s\n", entry.name);
        }
    }
}

