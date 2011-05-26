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

#define NUM_COMMANDS 6

static const Command* nextCommand(const char* prompt);

static const Command* findCommand(char* commandString);

static void addToHistory(const char* commandString);

static void updateCursor(size_t promptLen, int delta);

static int replaceInput(size_t promptLen, char* buffer);

static void printPrompt(const char* prompt);

static void autoComplete(const char* prompt);

static void addToInput(size_t promptLen, const char* in, size_t len);

static void chooseCurrentEntry(void);

const Command commands[] = {
    { &echo, "echo", "Prints the arguments passed to screen.", &manEcho },
    { &man, "man", "Display information about command execution.", &manMan },
    { &help, "help", "This command.", &manHelp },
    { &sudoku, "sudoku", "Play a game of Sudoku.", &manSudoku },
    { &calc, "calc", "Use a simple calculator.", &manCalc},
    { &fortune, "fortune", "Receive awesome knowledge.", &manFortune}
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

    // We always need to set the status needed by the shell, and then reset
    // it to the default, to make sure the input behaviour is as expected.
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

void updateCursor(size_t promptLen, int destPos) {

    // We always have to be extra careful with the fact
    // that we need to adjust our 0 start pos to 1 start pos
    int cursorPos = cur->cursor + promptLen + 1;
    destPos += promptLen + 1;

    if (destPos == cursorPos) {
        return;
    }

    int lineDelta = destPos / LINE_WIDTH - cursorPos / LINE_WIDTH;
    if (lineDelta > 0) {
        printf("\033[%dE", lineDelta);
    } else if (lineDelta < 0) {
        printf("\033[%dF", -lineDelta);
    }

    moveCursorInRow(((destPos - 1) % LINE_WIDTH) + 1);
    cur->cursor = destPos - promptLen - 1;
}

/**
 * Replace the current input with the content of buffer, on screen.
 *
 * @param promptLen The lenght of the prompt.
 * @param buffer The buffer to be printed.
 *
 * @return the size of the buffer printed.
 */
int replaceInput(size_t promptLen, char* buffer) {

    updateCursor(promptLen, -1);
    clearLine(ERASE_RIGHT);
    clearScreen(CLEAR_BELOW);
    updateCursor(promptLen, 0);

    printf("%s", buffer);

    return strlen(buffer);
}

const Command* nextCommand(const char* prompt) {

    size_t promptLen = strlen(prompt) + 7;
    int i;
    char in;

    // We reset the input status.
    cur->cursor = 0;
    cur->inputEnd = 0;
    cur->usingHistory = 0;

    printPrompt(prompt);
    // We set the whole string to null for safety.
    memset(cur->buffer, 0, BUFFER_SIZE);
    while ((in = getchar()) != '\n') {

        // Check for control sequences, these are things like arrow keys and such.
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

                            cur->inputEnd = replaceInput(
                                promptLen, history->input[history->current]
                            );
                            cur->cursor = cur->inputEnd;

                            cur->usingHistory = 1;
                        }
                        break;
                    case 'B':
                        // Down
                        if (cur->usingHistory) {

                            if (history->current == history->end) {
                                cur->usingHistory = 0;
                                cur->inputEnd = replaceInput(promptLen, cur->buffer);
                            } else {
                                history->current = (history->current + 1) % HISTORY_SIZE;
                                cur->inputEnd = replaceInput(
                                    promptLen, history->input[history->current]
                                );
                            }
                            cur->cursor = cur->inputEnd;
                        }
                        break;
                    case 'C':
                        // Right
                        if (cur->cursor == cur->inputEnd) {
                            break;
                        }

                        updateCursor(promptLen, cur->cursor + 1);
                        break;
                    case 'D':
                        // Left
                        if (cur->cursor == 0) {
                            break;
                        }

                        updateCursor(promptLen, cur->cursor - 1);
                        break;
                    case 'H':
                        // Home
                        updateCursor(promptLen, 0);
                        break;
                    case 'F':
                        // End
                        updateCursor(promptLen, cur->inputEnd);
                        break;
                    case CSI:
                        //This is a function key
                        in = getchar();
                        currentShellNumber = (in - 'A') % NUM_SHELLS;
                        // This tells the tty driver to change shell
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
                chooseCurrentEntry();
            }

            autoComplete(prompt);

        } else if (in == '\b') {

            if (cur->usingHistory) {
                chooseCurrentEntry();
            }

            if (cur->cursor > 0) {
                int destPos = cur->cursor - 1;
                cur->inputEnd--;

                // Move back once to step on the previous text
                updateCursor(promptLen, cur->cursor - 1);

                for (i = cur->cursor; i < cur->inputEnd; i++) {
                    cur->buffer[i] = cur->buffer[i + 1];
                }

                // Set a space in the end to make sure we erase previous text
                cur->buffer[cur->inputEnd] = ' ';

                // Print out
                printf("%s", cur->buffer + cur->cursor);

                // The input actually ends one after (the space we inserted)
                cur->cursor = cur->inputEnd + 1;
                updateCursor(promptLen, destPos);

                // Make sure the buffer is always null terminated
                cur->buffer[cur->inputEnd] = 0;
            }

        } else if (!isspace(in) || in == ' ') {

            if (cur->usingHistory) {
                chooseCurrentEntry();
            }

            addToInput(promptLen, &in, 1);
       }
    }

    if (cur->usingHistory) {
        // This means enter was pressed while browsing the history
        // So let's take the current history entry as the input
        memcpy(cur->buffer, history->input[history->current], BUFFER_SIZE);
    }

    updateCursor(promptLen, cur->inputEnd);
    putchar('\n');
    addToHistory(cur->buffer);

    return findCommand(cur->buffer);
}

void addToInput(size_t promptLen, const char* in, size_t len) {

    int i, destPos;
    // We only write when we have enough space
    if (cur->inputEnd + len < BUFFER_SIZE - 1) {

        // Move the characters after the cursor to make room for the new text
        for (i = cur->inputEnd - 1; i >= cur->cursor; i--) {
            cur->buffer[i + len] = cur->buffer[i];
        }

        // Copy the new input to the buffer
        for (i = 0; i < len; i++) {
            cur->buffer[cur->cursor + i] = in[i];
        }

        // Print the buffer starting from cursor and update the cursor to reflect this
        printf("%s", cur->buffer + cur->cursor);

        destPos = cur->cursor + len;

        cur->inputEnd += len;
        cur->cursor = cur->inputEnd;

        // Now move the cursor to where it ought to be
        updateCursor(promptLen, destPos);
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

    // Find the end of the first word
    for (len = 0; len < BUFFER_SIZE && commandString[len] != ' ' && commandString[len] != 0; len++);

    if (len > 0) {

        // Compare the first word to every command available
        for (i = 0; i < NUM_COMMANDS; i++) {
            res = &commands[i];
            if (strncmp(res->name, commandString, len) == 0) {
                // We found one :D Let's just return it
                return res;
            }
        }
    }

    // Ooops, no such command, let's let the user know.
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
    size_t promptLen, addedLen;
    char* fragment;

    // Let's find the start of the word before the cursor
    for (i = cur->cursor - 1; i >= 0 && cur->buffer[i] != ' '; i--);
    if (i == -1) {
        // We're at the first word
        i = 0;
    } else if (i != cur->inputEnd) {
        // This isn't the first word, so the for loop will end at the space
        // We need to skip that space, and take what's after
        i++;
    }

    len = cur->cursor - i;
    fragment = cur->buffer + i;

    if (len == 0) {
        // Every command is fair game
        candidates = NUM_COMMANDS;
    } else {

        // Count possible matches
        // Keep record of the last one, incase only one is found
        for (i = 0; i < NUM_COMMANDS; i++) {
            if (strncmp(fragment, commands[i].name, len) == 0) {
                candidates++;
                last = i;
            }
        }
    }

    promptLen = strlen(prompt) + 7;
    if (candidates == 0) {
        //TODO: System speaker?
        return;
    } else if (candidates == 1) {

        //Complete it :D
        addedLen = strlen(commands[last].name) - len;

        // If the addedLen is 0 then the word is already complete! There's nothng to add
        if (addedLen != 0) {
            addToInput(promptLen, commands[last].name + len, addedLen);
        }
    } else {

        int cursorPos = cur->cursor;

        // Show a list of possibles, using one line per 2 commands
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

        // We need to reprint the prompt and reposition the cursor afterwards
        printPrompt(prompt);
        printf("%s", cur->buffer);

        cur->cursor = cur->inputEnd;
        updateCursor(promptLen, cursorPos);
    }
}

static void chooseCurrentEntry(void) {
    memcpy(cur->buffer, history->input[history->current], BUFFER_SIZE);
    history->current = history->end;
    cur->usingHistory = 0;
}

