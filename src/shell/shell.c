#include "shell/shell.h"
#include "shell/info.h"
#include "system/call/ioctl/keyboard.h"
#include "library/stdio.h"
#include "library/string.h"
#include "library/stdlib.h"
#include "library/ctype.h"
#include "shell/commands.h"
#include "mcurses/mcurses.h"

#define BUFFER_SIZE 250
#define HISTORY_SIZE 50

#define NUM_COMMANDS 4

static void nextCommand(char* inputBuffer, const char* prompt);

static const Command* findCommand(char* commandString);

static void addToHistory(const char* commandString);

static int updateCursor(size_t promptLen, int cursorPos, int delta);

static int useHistory(size_t promptLen, int cursorPos, char* historyBuffer);

static void printPrompt(const char* prompt);

const Command commands[] = {
    { &echo, "echo", "Prints the arguments passed to screen.", &manEcho },
    { &man, "man", "Display information about command execution.", &manMan },
    { &help, "help", "This command.", &manHelp },
    { &sudoku, "sudoku", "Play a game of Sudoku.", &manSudoku }
};

struct {
    char input[HISTORY_SIZE][BUFFER_SIZE];
    int start;
    int end;
    int current;
    char before[BUFFER_SIZE];
} history;

static termios inputStatus;
static const termios shellStatus = { 0, 0 };

void shell(void) {

    const Command* cmd;
    static char inputBuffer[BUFFER_SIZE];

    history.start = 0;
    history.end = 0;
    history.current = 0;

    ioctl(0, TCGETS, (void*) &inputStatus);
    ioctl(0, TCSETS, (void*) &shellStatus);

    while (1) {

        nextCommand(inputBuffer, "> ");
        cmd = findCommand(inputBuffer);
        if (cmd != NULL) {

            ioctl(0, TCSETS, (void*) &inputStatus);
            cmd->func(inputBuffer);
            ioctl(0, TCGETS, (void*) &inputStatus);

            ioctl(0, TCSETS, (void*) &shellStatus);
        }
    }
}

void printPrompt(const char* prompt) {
    setForegroundColor(COLOR_GREEN);
    printf("%s", prompt);
    setForegroundColor(COLOR_WHITE);
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

int useHistory(size_t promptLen, int cursorPos, char* historyBuffer) {

    updateCursor(promptLen - 1, cursorPos + 1, - cursorPos - 1);
    clearLine(ERASE_RIGHT);
    clearScreen(CLEAR_BELOW);
    updateCursor(promptLen - 1, 0, 1);

    printf("%s", historyBuffer);

    return strlen(historyBuffer);
}

void nextCommand(char* inputBuffer, const char* prompt) {

    int cursorPos = 0, inputEnd = 0, i, usingHistory = 0;
    size_t promptLen = strlen(prompt);
    char in;

    printPrompt(prompt);
    memset(inputBuffer, 0, BUFFER_SIZE);
    while ((in = getchar()) != '\n') {

        if (in == '\033') {
            if (getchar() == CSI) {
                // We know this! Yay!
                switch (getchar()) {
                    case 'A':
                        // Up
                        if (history.current != history.start) {
                            history.current--;
                            if (history.current < 0) {
                                history.current = HISTORY_SIZE - 1;
                            }

                            inputEnd = useHistory(promptLen, cursorPos, history.input[history.current]);
                            cursorPos = inputEnd;

                            usingHistory = 1;
                        }
                        break;
                    case 'B':
                        // Down
                        if (usingHistory) {

                            if (history.current == history.end) {
                                usingHistory = 0;
                                inputEnd = useHistory(promptLen, cursorPos, inputBuffer);
                            } else {
                                history.current = (history.current + 1) % HISTORY_SIZE;
                                inputEnd = useHistory(promptLen, cursorPos, history.input[history.current]);
                            }
                            cursorPos = inputEnd;
                        }
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
                    case CSI:
                        //This is a function key
                        in = getchar();
                        printf("\033[%dZ", in - 'A');
                        //TODO: Change shell
                        break;
                }
            }
        } else if (in == '\b') {

            if (usingHistory) {
                memcpy(inputBuffer, history.input[history.current], BUFFER_SIZE);
                history.current = history.end;
                usingHistory = 0;
            }

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

            if (usingHistory) {
                memcpy(inputBuffer, history.input[history.current], BUFFER_SIZE);
                history.current = history.end;
                usingHistory = 0;
            }

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

    if (usingHistory) {
        memcpy(inputBuffer, history.input[history.current], BUFFER_SIZE);
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

const Command* findCommand(char* commandString) {

    const Command* res;
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

    commandString[len + 1] = 0;
    printf("Command not found: %s\n", commandString);

    return NULL;
}

const Command* getShellCommands(size_t* len) {
    *len = NUM_COMMANDS;
    return commands;
}

