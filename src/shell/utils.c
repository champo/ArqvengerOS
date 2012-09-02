#include "shell/utils.h"
#include "library/stdio.h"
#include "system/call/ioctl/keyboard.h"

termios oldTermios;
termios passwdTermios = {1 , 0};


void askForPasswd(const char* prompt, char* passwd) {

    if (prompt != NULL) {
        printf(prompt);
    }

    ioctl(0, TCGETS, (void*) &oldTermios);
    ioctl(0, TCSETS, (void*) &passwdTermios);

    askForInput(NULL, passwd);

    ioctl(0, TCSETS, (void*) &oldTermios);
    printf("\n");

    return;
}


void askForInput(const char* prompt, char* buffer) {

    if (prompt != NULL) {
        printf(prompt);
    }

    scanf("%s", buffer);
    cleanbuffer();

    return;
}
