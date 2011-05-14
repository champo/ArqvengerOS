#include "shell/shell.h"
#include "system/call/ioctl/keyboard.h"
#include "library/stdio.h"
#include "library/string.h"
#include "library/stdlib.h"
#include "library/ctype.h"
#include "shell/commands.h"
#include "mcurses/mcurses.h"

#define BUFFER_SIZE 250
#define HISTORY_SIZE 50

#define NUM_COMMANDS 1

typedef void (*commandFunction)(char* argv);

typedef struct {
    commandFunction func;
    const char* name;
} command;

void nextCommand(char* inputBuffer, const char* prompt);

const command* findCommand(char* commandString);

void addToHistory(const char* commandString);

int updateCursor(size_t promptLen, int cursorPos, int delta);

static const command commands[] = {
    { &echo, "echo" }
};

static struct {
    char input[HISTORY_SIZE][BUFFER_SIZE];
    int start;
    int end;
    int current;
} history;

static termios inputStatus;
static const termios shellStatus = { 0, 0 };

void shell(void) {

    const command* cmd;
    static char inputBuffer[BUFFER_SIZE];

    history.start = 0;
    history.end = 0;
    history.current = 0;

    ioctl(0, TCGETS, &inputStatus);
    ioctl(0, TCSETS, &shellStatus);

    while (1) {

        nextCommand(inputBuffer, "> ");
        cmd = findCommand(inputBuffer);
        if (cmd != NULL) {

            ioctl(0, TCSETS, &inputStatus);
            cmd->func(inputBuffer);
            ioctl(0, TCGETS, &inputStatus);

            ioctl(0, TCSETS, &shellStatus);
        }
    }
}

int updateCursor(size_t promptLen, int cursorPos, int delta) {

    if (delta == 0) {
        return cursorPos;
    }

    cursorPos += promptLen;
    int destPos = cursorPos + delta;
    if (destPos / LINE_WIDTH == cursorPos / LINE_WIDTH) {

        if (delta > 0) {
            printf("\033[%dC", delta);
        } else {
            printf("\033[%dD", -delta);
        }
    } else {

        int lineDelta = destPos / LINE_WIDTH - cursorPos / LINE_WIDTH;
        if (delta > 0) {
            printf("\033[%dE\033[%dC", lineDelta, ((destPos - 1) % LINE_WIDTH) + 1);
        } else {
            printf("\033[%dF\033[%dC", -lineDelta, ((destPos - 1) % LINE_WIDTH) + 1);
        }
    }

    return destPos - promptLen;
}

void nextCommand(char* inputBuffer, const char* prompt) {

    int cursorPos = 0, inputEnd = 0, i;
    size_t promptLen = strlen(prompt);
    char in;

    printf("%s", prompt);
    memset(inputBuffer, 0, BUFFER_SIZE);
    while ((in = getchar()) != '\n') {

        if (in == '\033') {
            if (getchar() == '[') {
                // We know this! Yay!
                switch (getchar()) {
                    case 'A':
                        // Up
                        break;
                    case 'B':
                        // Down
                        break;
                    case 'C':
                        // Right
                        if (cursorPos == inputEnd) {
                            break;
                        }

                        cursorPos = updateCursor(promptLen, cursorPos, 1);
                        break;
                    case 'D':
                        // Left
                        if (cursorPos == 0) {
                            break;
                        }

                        cursorPos = updateCursor(promptLen, cursorPos, -1);
                        break;
                    case 'H':
                        // Home
                        cursorPos = updateCursor(promptLen, cursorPos, -cursorPos);
                        break;
                    case 'F':
                        // End
                        cursorPos = updateCursor(promptLen, cursorPos, inputEnd - cursorPos);
                        break;
                }
            }
        } else if (in == '\b') {

            if (cursorPos > 0) {
                inputEnd--;

                for (i = cursorPos - 1; i < inputEnd; i++) {
                    inputBuffer[i] = inputBuffer[i + 1];
                }
                inputBuffer[inputEnd] = ' ';

                // Move back once to step on the previous text
                cursorPos = updateCursor(promptLen, cursorPos, -1);
                printf("%s", inputBuffer + cursorPos);
                cursorPos = updateCursor(promptLen, inputEnd + 1, cursorPos - inputEnd - 1);

                inputBuffer[inputEnd] = 0;
            }
        } else if (!isspace(in) || in == ' ') {

            if (inputEnd + 1 < BUFFER_SIZE - 1) {

                for (i = inputEnd - 1; i >= cursorPos; i--) {
                    inputBuffer[i + 1] = inputBuffer[i];
                }
                inputBuffer[cursorPos] = in;
                inputEnd++;

                printf("%s", inputBuffer + cursorPos);
                cursorPos = updateCursor(promptLen, inputEnd, cursorPos - inputEnd + 1);
            }
        }
    }

    updateCursor(promptLen, cursorPos, inputEnd - cursorPos);
    putchar('\n');
    addToHistory(inputBuffer);
}

void addToHistory(const char* commandString) {

    strncpy(history.input[history.end], commandString, BUFFER_SIZE - 1);
    history.input[history.end][BUFFER_SIZE - 1] = 0;

    history.end = (history.end + 1) % HISTORY_SIZE;
    history.current = history.end;

    if (history.start == history.end) {
        history.start = (history.start + 1) % HISTORY_SIZE;
    }
}

const command* findCommand(char* commandString) {

    const command* res;
    size_t i, len;

    for (i = 0; i < BUFFER_SIZE - 1 && commandString[i] != ' ' && commandString[i] != 0; i++);
    if (commandString[i] == ' ') {
        len = i - 1;
    } else {
        len = i;
    }

    for (i = 0; i < NUM_COMMANDS; i++) {
        res = &commands[i];
        if (strncmp(res->name, commandString, len) == 0) {
            return res;
        }
    }

    commandString[len] = 0;
    printf("Command not found: %s\n", commandString);

    return NULL;
}

