#include "shell/shell.h"
#include "shell/info.h"
#include "system/call/ioctl/keyboard.h"
#include "library/stdio.h"
#include "library/string.h"
#include "library/stdlib.h"
#include "library/ctype.h"
#include "shell/commands.h"
#include "mcurses/mcurses.h"

#define BUFFER_SIZE 500
#define HISTORY_SIZE 50

#define NUM_SHELLS 4

#define NUM_COMMANDS 4

static const Command* nextCommand(const char* prompt);

static const Command* findCommand(char* commandString);

static void addToHistory(const char* commandString);

static int updateCursor(size_t promptLen, int cursorPos, int delta);

static int useHistory(size_t promptLen, int cursorPos, char* historyBuffer);

static void printPrompt(const char* prompt);

static void autoComplete(const char* prompt);

static void addToInput(size_t promptLen, const char* in, size_t len);

const Command commands[] = {
    { &echo, "echo", "Prints the arguments passed to screen.", &manEcho },
    { &man, "man", "Display information about command execution.", &manMan },
    { &help, "help", "This command.", &manHelp },
    { &sudoku, "sudoku", "Play a game of Sudoku.", &manSudoku }
};

typedef struct {
    char input[HISTORY_SIZE][BUFFER_SIZE];
    int start;
    int end;
    int current;
    char before[BUFFER_SIZE];
} History;

typedef struct {
    History history;
    char buffer[BUFFER_SIZE];
    int inputEnd;
    int cursor;
    int usingHistory;
    int started;
    termios inputStatus;
} Shell;

static const termios shellStatus = { 0, 0 };
static Shell shells[NUM_SHELLS];
static int currentShellNumber;
static History* history;
static Shell* cur;

void shell(void) {

    const Command* cmd;
    int i;
    for (i = NUM_SHELLS - 1; i >= 0; i--) {
        cur = &shells[i];
        history = &cur->history;
        cur->history.start = 0;
        cur->history.end = 0;
        cur->history.current = 0;
        cur->started = 0;
    }

    currentShellNumber = 0;
    cur->started = 1;

    ioctl(0, TCGETS, (void*) &cur->inputStatus);
    ioctl(0, TCSETS, (void*) &shellStatus);

    while (1) {

        cmd = nextCommand("guest");
        if (cmd != NULL) {

            ioctl(0, TCSETS, (void*) &cur->inputStatus);
            cmd->func(cur->buffer);
            ioctl(0, TCGETS, (void*) &cur->inputStatus);

            ioctl(0, TCSETS, (void*) &shellStatus);
        }
    }
}

void printPrompt(const char* prompt) {
    setForegroundColor(COLOR_GREEN);
    printf("%s@tty%d >", prompt, currentShellNumber);
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

const Command* nextCommand(const char* prompt) {

    size_t promptLen = strlen(prompt) + 7;
    int i;
    char in;

    cur->cursor = 0;
    cur->inputEnd = 0;
    cur->usingHistory = 0;

    printPrompt(prompt);
    memset(cur->buffer, 0, BUFFER_SIZE);
    while ((in = getchar()) != '\n') {

        if (in == '\033') {
            if (getchar() == CSI) {
                // We know this! Yay!
                switch (getchar()) {
                    case 'A':
                        // Up
                        if (history->current != history->start) {
                            history->current--;
                            if (history->current < 0) {
                                history->current = HISTORY_SIZE - 1;
                            }

                            cur->inputEnd = useHistory(promptLen, cur->cursor, history->input[history->current]);
                            cur->cursor = cur->inputEnd;

                            cur->usingHistory = 1;
                        }
                        break;
                    case 'B':
                        // Down
                        if (cur->usingHistory) {

                            if (history->current == history->end) {
                                cur->usingHistory = 0;
                                cur->inputEnd = useHistory(promptLen, cur->cursor, cur->buffer);
                            } else {
                                history->current = (history->current + 1) % HISTORY_SIZE;
                                cur->inputEnd = useHistory(promptLen, cur->cursor, history->input[history->current]);
                            }
                            cur->cursor = cur->inputEnd;
                        }
                        break;
                    case 'C':
                        // Right
                        if (cur->cursor == cur->inputEnd) {
                            break;
                        }

                        cur->cursor = updateCursor(promptLen, cur->cursor, 1);
                        break;
                    case 'D':
                        // Left
                        if (cur->cursor == 0) {
                            break;
                        }

                        cur->cursor = updateCursor(promptLen, cur->cursor, -1);
                        break;
                    case 'H':
                        // Home
                        cur->cursor = updateCursor(promptLen, cur->cursor, -cur->cursor);
                        break;
                    case 'F':
                        // End
                        cur->cursor = updateCursor(promptLen, cur->cursor, cur->inputEnd - cur->cursor);
                        break;
                    case CSI:
                        //This is a function key
                        in = getchar();
                        currentShellNumber = (in - 'A') % NUM_SHELLS;
                        printf("\033[%dZ", currentShellNumber);

                        cur = &shells[currentShellNumber];
                        history = &cur->history;

                        if (cur->started == 0) {
                            cur->started = 1;
                            return NULL;
                        }
                        break;
                }
            }
        } else if (in == '\t') {

            if (cur->usingHistory) {
                memcpy(cur->buffer, history->input[history->current], BUFFER_SIZE);
                history->current = history->end;
                cur->usingHistory = 0;
            }

            autoComplete(prompt);

        } else if (in == '\b') {

            if (cur->usingHistory) {
                memcpy(cur->buffer, history->input[history->current], BUFFER_SIZE);
                history->current = history->end;
                cur->usingHistory = 0;
            }

            if (cur->cursor > 0) {
                cur->inputEnd--;

                for (i = cur->cursor - 1; i < cur->inputEnd; i++) {
                    cur->buffer[i] = cur->buffer[i + 1];
                }
                cur->buffer[cur->inputEnd] = ' ';

                // Move back once to step on the previous text
                cur->cursor = updateCursor(promptLen, cur->cursor, -1);
                printf("%s", cur->buffer + cur->cursor);
                cur->cursor = updateCursor(promptLen, cur->inputEnd + 1, cur->cursor - cur->inputEnd - 1);

                cur->buffer[cur->inputEnd] = 0;
            }

        } else if (!isspace(in) || in == ' ') {

            if (cur->usingHistory) {
                memcpy(cur->buffer, history->input[history->current], BUFFER_SIZE);
                history->current = history->end;
                cur->usingHistory = 0;
            }

            addToInput(promptLen, &in, 1);
       }
    }

    if (cur->usingHistory) {
        memcpy(cur->buffer, history->input[history->current], BUFFER_SIZE);
    }

    updateCursor(promptLen, cur->cursor, cur->inputEnd - cur->cursor);
    putchar('\n');
    addToHistory(cur->buffer);

    return findCommand(cur->buffer);
}

void addToInput(size_t promptLen, const char* in, size_t len) {

    int i;
    if (cur->inputEnd + len < BUFFER_SIZE - 1) {

        for (i = cur->inputEnd - 1; i >= cur->cursor; i--) {
            cur->buffer[i + len] = cur->buffer[i];
        }

        for (i = 0; i < len; i++) {
            cur->buffer[cur->cursor + i] = in[i];
        }
        cur->inputEnd += len;

        printf("%s", cur->buffer + cur->cursor);
        cur->cursor = updateCursor(promptLen, cur->inputEnd, cur->cursor - cur->inputEnd + 1);
    }
}

void addToHistory(const char* commandString) {

    strncpy(history->input[history->end], commandString, BUFFER_SIZE - 1);
    history->input[history->end][BUFFER_SIZE - 1] = 0;

    history->end = (history->end + 1) % HISTORY_SIZE;
    history->current = history->end;

    if (history->start == history->end) {
        history->start = (history->start + 1) % HISTORY_SIZE;
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
        if (strncmp(res->name, commandString, strlen(res->name)) == 0) {
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

void autoComplete(const char* prompt) {

    int i, candidates = 0, len, last = 0;
    char* fragment;

    for (i = cur->cursor - 1; i >= 0 && cur->buffer[i] != ' '; i--);
    if (i == -1) {
        i = 0;
    }

    len = cur->cursor - i;
    fragment = cur->buffer + i;

    if (len == 0) {
        candidates = NUM_COMMANDS;
    } else {

        for (i = 0; i < NUM_COMMANDS; i++) {
            if (strncmp(fragment, commands[i].name, len) == 0) {
                candidates++;
                last = i;
            }
        }
    }

    if (candidates == 0) {
        //TODO: System speaker?
        return;
    } else if (candidates == 1) {
        //Complete it :D
        addToInput(strlen(prompt) + 7, commands[last].name + len, strlen(commands[last].name) - len);
    } else {
        // Show a list of possibles
        candidates = 0;
        for (i = 0; i < NUM_COMMANDS; i++) {
            if (strncmp(fragment, commands[i].name, len) == 0) {
                if ((candidates % 2) == 0) {
                    putchar('\n');
                    moveCursorInRow(4);
                } else {
                    moveCursorInRow(LINE_WIDTH / 2 + 4);
                }

                candidates++;
                printf("%s", commands[i].name);
            }
        }
        putchar('\n');
        printPrompt(prompt);
        printf("%s", cur->buffer);
        updateCursor(strlen(prompt) + 7, cur->inputEnd, cur->cursor - cur->inputEnd);
    }
}

