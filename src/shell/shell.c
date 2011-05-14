#include "shell/shell.h"
#include "system/call.h"
#include "library/stdio.h"
#include "library/string.h"
#include "library/stdlib.h"
#include "shell/commands.h"

#define BUFFER_SIZE 500

#define NUM_COMMANDS 1

typedef void (*commandFunction)(char* argv);

typedef struct {
    commandFunction func;
    const char* name;
} command;

void nextCommand(char* inputBuffer);

const command* findCommand(const char* commandString);

static const command commands[] = {
    { &echo, "echo" }
};

void shell(void) {

    const command* cmd;
    static char inputBuffer[BUFFER_SIZE];
    while (1) {
        putchar('>');
        putchar(' ');
        nextCommand(inputBuffer);
        cmd = findCommand(inputBuffer);
        if (cmd != NULL) {
            cmd->func(inputBuffer);
        }
    }
}

void nextCommand(char* inputBuffer) {

    int bufferPos = 0, in;
    char a[1];

    //TODO: Use getchar
    while ((in = read(0, a, 1)) && a[0] != '\n') {
        if (bufferPos < BUFFER_SIZE) {
            inputBuffer[bufferPos++] = a[0];
        }
    }
    inputBuffer[bufferPos] = 0;
}

const command* findCommand(const char* commandString) {

    const command* res;
    int i;

    for (i = 0; i < NUM_COMMANDS; i++) {
        res = &commands[i];
        if (strcmp(res->name, commandString) <= 0) {
            return res;
        }
    }

    //TODO: ERRRO!

    return NULL;
}

