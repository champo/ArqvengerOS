#include "shell/shell.h"
#include "shell/info.h"
#include "system/call/ioctl/keyboard.h"
#include "system/accessControlList/users.h"
#include "library/stdio.h"
#include "library/string.h"
#include "library/sys.h"
#include "library/stdlib.h"
#include "library/ctype.h"
#include "shell/commands.h"
#include "mcurses/mcurses.h"


#define BUFFER_SIZE 500
#define HISTORY_SIZE 50

struct History {
    char input[HISTORY_SIZE][BUFFER_SIZE];
    int start;
    int end;
    int current;
    char before[BUFFER_SIZE];
};

struct Shell {
    struct History history;
    char buffer[BUFFER_SIZE];
    int inputEnd;
    int cursor;
    int usingHistory;
    int ttyNumber;
    termios inputStatus;
};

static const Command* nextCommand(struct Shell* self, const char* prompt);

static const Command* findCommand(char* commandString);

static void addToHistory(struct History* history, const char* commandString);

static void updateCursor(struct Shell* self, size_t promptLen, int delta);

static int replaceInput(struct Shell* self, size_t promptLen, char* buffer);

static void printPrompt(struct Shell* self, const char* prompt);

static void autoComplete(struct Shell* self, const char* prompt);

static void addToInput(struct Shell* self, size_t promptLen, const char* in, size_t len);

static void chooseCurrentEntry(struct Shell* self);

static void run_command(struct Shell* self, const Command* cmd);

#define NUM_COMMANDS 39
static Command commands[] = {
    { &echo, "echo", "Prints the arguments passed to screen.", &manEcho, 0 },
    { &man, "man", "Display information about command execution.", &manMan, 1 },
    { &help, "help", "This command.", &manHelp, 1 },
    { &sudoku, "sudoku", "Play a game of Sudoku.", &manSudoku, 0 },
    { &calc, "calc", "Use a simple calculator", &manCalc, 0},
    { &getCPUSpeed, "getCPUSpeed", "Get the CPU speed", &manGetCPUSpeed, 0},
    { &fortune, "fortune", "Receive awesome knowledge.", &manFortune, 0},
    { &date, "date", "Display current date.", &manDate, 0},
    { &killCmd, "kill", "Kill a running process.", &manKill, 0},
    { &top, "top", "Display information about running processes.", &manTop, 0},
    { &cat, "cat", "Output the contents of a file.", &man_cat, 0},
    { &roflcopter, "roflcopter", "We need a ROFLcopter.", &manRoflcopter, 0},
    { &command_cd, "cd", "Change the current working directory.", &man_cd, 1},
    { &command_pwd, "pwd", "Print the current working directory.", &man_pwd, 1},
    { &command_mkdir, "mkdir", "Create a new directory.", &manMkdir, 0},
    { &command_rmdir, "rmdir", "Remove an empty directory.", &manRmdir, 0},
    { &command_ls, "ls", "List entries in a directory.", &manLs, 0},
    { &append, "append", "Append content to a file.", &man_append, 0},
    { &adduser, "adduser", "Add new user account.", &manAdduser, 0},
    { &users, "users", "Show users logged.", &manUsers, 0},
    { &userdel, "userdel", "Delete an user account and related files.", &manUserdel, 0},
    { &passwd, "passwd", "Chanfe user password", &manPasswd, 0},
    { &groupadd, "groupadd", "Create a new group.", &manGroupadd, 0},
    { &groups, "groups", "Display current group names.", &manGroups, 0},
    { &groupdel, "groupdel", "Delete a group", &manGroupdel, 0},
    { &command_ln, "ln", "Create a symbolic link.", &man_ln, 0},
    { &command_unlink, "unlink", "Remove a specified file.", &manUnlink, 0},
    { &command_mkfifo, "mkfifo", "Creates a named pipe.", &manFifo, 0},
    { &command_chmod, "chmod", "Change the permissions of a file.", &man_chmod, 0},
    { &command_renice, "renice", "Alter priority of a running process.", &manRenice, 0},
    { &busywait, "busywait", "Busywaits eternally, consuming resources and CPU time.", &manBusywait, 0},
    { &logout, "logout", "Logs out of the shell.", &manLogout, 1},
    { &command_chown, "chown", "Changes the user and group of a file.", &man_chown, 0},
    { &command_cp, "cp", "Copy a file or directory.", &man_cp, 0},
    { &command_mv, "mv", "Move (rename) files.", &man_mv, 0},
    { &command_log, "loglevel", "Change the level of the log console.", &man_log, 0},
    { &stacksize, "stacksize", "Create a process that will grow its stack as indicated.", &manstacksize, 0},
    { &rma, "rma", "Access a random memory location.", &man_rma, 0},
    { &atto, "atto", "Minimal file editor.", &man_atto, 0}
};

static termios shellStatus = { 0, 0 };

/**
 * Shell entry point.
 */
void shell(char* unused) {

    const Command* cmd;
    struct Shell me;
    struct Shell* self = &me;
    self->history.start = 0;
    self->history.end = 0;
    self->history.current = 0;

    //TODO: Get this from somewhere
    self->ttyNumber = 0;

    // We always need to set the status needed by the shell, and then reset
    // it to the default, to make sure the input behaviour is as expected.
    ioctl(0, TCGETS, (void*) &self->inputStatus);
    ioctl(0, TCSETS, (void*) &shellStatus);

    int uid,gid;
    struct User* user;
    char* username;

    getProcessPersona(getpid(), &uid, &gid);
    user = get_user_by_id(uid);
    username = user->name;
    free_user(user);

    sort_commands(commands);

    while (1) {

        cmd = nextCommand(self, username);
        if (cmd != NULL) {

            ioctl(0, TCSETS, (void*) &self->inputStatus);
            run_command(self, cmd);
            ioctl(0, TCGETS, (void*) &self->inputStatus);

            ioctl(0, TCSETS, (void*) &shellStatus);
        }
    }
}


/**
 * Simple bubble sort implementation to sort the arrat of commands alphabetically.
 *
 * @param commands The array of Command to be sorted.
 */
void sort_commands(Command* commands) {

    int swapped = 1;
    int n = NUM_COMMANDS;

    while (swapped) {
        swapped = 0;
        for (int i = 1; i < n; i++) {

            if (strcmp(commands[i - 1].name, commands[i].name) > 0) {
                Command aux = commands[i - 1];
                commands[i - 1] = commands[i];
                commands[i] = aux;
                swapped = 1;
            }
        }
        n--;
    }

    return;
}



void run_command(struct Shell* self, const Command* cmd) {

    if (cmd->internal) {
        cmd->func(self->buffer);
    } else {
        int fg = 1, end = self->inputEnd - 1;
        do {
            if (self->buffer[end] == '&') {
                fg = 0;
                break;
            }
        } while (end > 0 && self->buffer[end--] == ' ');

        pid_t child = run(cmd->func, self->buffer, fg);
        if (fg) {
            while (child != wait());
        }
    }
}

/**
 * Print the prompt.
 *
 * @param prompt The prompt to print.
 */
void printPrompt(struct Shell* self, const char* prompt) {
    setForegroundColor(COLOR_GREEN);
    printf("%s@tty%d >", prompt, self->ttyNumber);
    setForegroundColor(COLOR_WHITE);
}

/**
 * Update the cursor position to a new value.
 *
 * @param promptLen The len of the prompt.
 * @param destPos The position to set the cursor to.
 */
void updateCursor(struct Shell* self, size_t promptLen, int destPos) {

    // We always have to be extra careful with the fact
    // that we need to adjust our 0 start pos to 1 start pos
    int cursorPos = self->cursor + promptLen;
    destPos += promptLen;

    if (destPos == cursorPos) {
        return;
    }

    int lineDelta = destPos / LINE_WIDTH - cursorPos / LINE_WIDTH;
    if (lineDelta > 0) {
        printf("\033[%dE", lineDelta);
    } else if (lineDelta < 0) {
        printf("\033[%dF", -lineDelta);
    }

    moveCursorInRow((destPos % LINE_WIDTH) + 1);
    self->cursor = destPos - promptLen;
}

/**
 * Replace the current input with the content of buffer, on screen.
 *
 * @param promptLen The lenght of the prompt.
 * @param buffer The buffer to be printed.
 *
 * @return the size of the buffer printed.
 */
int replaceInput(struct Shell* self, size_t promptLen, char* buffer) {

    updateCursor(self, promptLen, -1);
    clearLine(ERASE_RIGHT);
    clearScreen(CLEAR_BELOW);
    updateCursor(self, promptLen, 0);

    printf("%s", buffer);

    return strlen(buffer);
}

/**
 * Parse & find the next command.
 *
 * @param prompt The prompt to print.
 *
 * @return The command to execute, if one was found, NULL otherwise.
 */
const Command* nextCommand(struct Shell* self, const char* prompt) {

    size_t promptLen = strlen(prompt) + 7;
    int i;
    char in;

    // We reset the input status.
    self->cursor = 0;
    self->inputEnd = 0;
    self->usingHistory = 0;

    struct History* history = &self->history;

    printPrompt(self, prompt);
    // We set the whole string to null for safety.
    memset(self->buffer, 0, BUFFER_SIZE);
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

                            self->inputEnd = replaceInput(
                                self, promptLen, history->input[history->current]
                            );
                            self->cursor = self->inputEnd;

                            self->usingHistory = 1;
                        }
                        break;
                    case 'B':
                        // Down
                        if (self->usingHistory) {

                            if (history->current == history->end) {
                                self->usingHistory = 0;
                                self->inputEnd = replaceInput(self, promptLen, self->buffer);
                            } else {
                                history->current = (history->current + 1) % HISTORY_SIZE;
                                self->inputEnd = replaceInput(
                                    self, promptLen, history->input[history->current]
                                );
                            }
                            self->cursor = self->inputEnd;
                        }
                        break;
                    case 'C':
                        // Right
                        if (self->cursor == self->inputEnd) {
                            break;
                        }

                        updateCursor(self, promptLen, self->cursor + 1);
                        break;
                    case 'D':
                        // Left
                        if (self->cursor == 0) {
                            break;
                        }

                        updateCursor(self, promptLen, self->cursor - 1);
                        break;
                    case 'H':
                        // Home
                        updateCursor(self, promptLen, 0);
                        break;
                    case 'F':
                        // End
                        updateCursor(self, promptLen, self->inputEnd);
                        break;
                    case CSI:
                        //This is a function key
                        in = getchar();
                        // We don't support this anymore (not from here at least)
                        break;
                }
            }
        } else if (in == '\t') {

            if (self->usingHistory) {
                chooseCurrentEntry(self);
            }

            autoComplete(self, prompt);

        } else if (in == '\b') {

            if (self->usingHistory) {
                chooseCurrentEntry(self);
            }

            if (self->cursor > 0) {
                int destPos = self->cursor - 1;
                self->inputEnd--;

                // Move back once to step on the previous text
                updateCursor(self, promptLen, self->cursor - 1);

                for (i = self->cursor; i < self->inputEnd; i++) {
                    self->buffer[i] = self->buffer[i + 1];
                }

                // Set a space in the end to make sure we erase previous text
                self->buffer[self->inputEnd] = ' ';

                // Print out
                printf("%s", self->buffer + self->cursor);

                // The input actually ends one after (the space we inserted)
                self->cursor = self->inputEnd + 1;
                updateCursor(self, promptLen, destPos);

                // Make sure the buffer is always null terminated
                self->buffer[self->inputEnd] = 0;
            }

        } else if (!isspace(in) || in == ' ') {

            if (self->usingHistory) {
                chooseCurrentEntry(self);
            }

            addToInput(self, promptLen, &in, 1);
       }
    }

    if (self->usingHistory) {
        // This means enter was pressed while browsing the history
        // So let's take the current history entry as the input
        memcpy(self->buffer, history->input[history->current], BUFFER_SIZE);
    }

    updateCursor(self, promptLen, self->inputEnd);
    putchar('\n');
    addToHistory(history, self->buffer);

    return findCommand(self->buffer);
}

/**
 * Add a string of length len to the input.
 *
 * @param promptLen The length of the prompt.
 * @param in The string to add. Doesn't need to be NULL terminated.
 * @param len The length of the string.
 */
void addToInput(struct Shell* self, size_t promptLen, const char* in, size_t len) {

    int i, destPos;
    // We only write when we have enough space
    if (self->inputEnd + len < BUFFER_SIZE - 1) {

        // Move the characters after the cursor to make room for the new text
        for (i = self->inputEnd - 1; i >= self->cursor; i--) {
            self->buffer[i + len] = self->buffer[i];
        }

        // Copy the new input to the buffer
        for (i = 0; i < len; i++) {
            self->buffer[self->cursor + i] = in[i];
        }

        // Print the buffer starting from cursor and update the cursor to reflect this
        printf("%s", self->buffer + self->cursor);

        destPos = self->cursor + len;

        self->inputEnd += len;
        self->cursor = self->inputEnd;

        // Now move the cursor to where it ought to be
        updateCursor(self, promptLen, destPos);
    }
}

/**
 * Add the input to the history.
 *
 * @param commandString The input to add.
 */
void addToHistory(struct History* history, const char* commandString) {

    if (strlen(commandString) == 0) {
        return;
    }

    strncpy(history->input[history->end], commandString, BUFFER_SIZE - 1);
    history->input[history->end][BUFFER_SIZE - 1] = 0;

    history->end = (history->end + 1) % HISTORY_SIZE;
    history->current = history->end;

    if (history->start == history->end) {
        history->start = (history->start + 1) % HISTORY_SIZE;
    }
}

/**
 * Parse the command string and find the command, if any, to execute.
 *
 * @param commandString The string to parse.
 *
 * @return The command to execute, NULL if none was found.
 */
const Command* findCommand(char* commandString) {

    const Command* res;
    size_t i, len;
    char oldChar;

    // Find the end of the first word
    for (len = 0; len < BUFFER_SIZE && commandString[len] != ' ' && commandString[len] != 0; len++);

    oldChar = commandString[len];
    commandString[len] = 0;
    if (len > 0) {

        // Compare the first word to every command available
        for (i = 0; i < NUM_COMMANDS; i++) {
            res = &commands[i];
            if (strcmp(res->name, commandString) == 0) {

                commandString[len] = oldChar;
                // We found one :D Let's just return it
                return res;
            }
        }
    }

    // Ooops, no such command, let's let the user know.

    printf("Command not found: %s\n", commandString);

    return NULL;
}

/**
 * Get the list of commands the shell knows.
 *
 * @param len A pointer to the size_t to store the number of commands.
 *
 * @return An array of commands
 */
const Command* getShellCommands(size_t* len) {
    *len = NUM_COMMANDS;
    return commands;
}

/**
 * Auto complete the word under the cursor.
 *
 * @param prompt The prompt to print.
 */
void autoComplete(struct Shell* self, const char* prompt) {

    int i, candidates = 0, len, last = 0;
    size_t promptLen, addedLen;
    char* fragment;

    // Let's find the start of the word before the cursor
    for (i = self->cursor - 1; i >= 0 && self->buffer[i] != ' '; i--);
    if (i == -1) {
        // We're at the first word
        i = 0;
    } else if (i != self->inputEnd) {
        // This isn't the first word, so the for loop will end at the space
        // We need to skip that space, and take what's after
        i++;
    }

    len = self->cursor - i;
    fragment = self->buffer + i;

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
            addToInput(self, promptLen, commands[last].name + len, addedLen);
        }
    } else {

        int cursorPos = self->cursor;

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
        printPrompt(self, prompt);
        printf("%s", self->buffer);

        self->cursor = self->inputEnd;
        updateCursor(self, promptLen, cursorPos);
    }
}

/**
 * Replace the input with the current history entry.
 */
void chooseCurrentEntry(struct Shell* self) {
    memcpy(self->buffer, self->history.input[self->history.current], BUFFER_SIZE);
    self->history.current = self->history.end;
    self->usingHistory = 0;
}

