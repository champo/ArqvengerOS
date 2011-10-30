#include "system/call.h"
#include "system/call/codes.h"
#include "system/malloc/malloc.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/limits.h"
#include "library/stdarg.h"
#include "library/call.h"
#include "type.h"
#include "library/ctype.h"
#include "constants.h"

//TODO remove this when changed
#include "system/common.h"
#include "system/mm.h"

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100
#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010
#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

FILE *stdout;
FILE *stdin;
FILE *stderr;


/**
 * Insert a character into the given stream.
 *
 * @param c, the character to be written.
 * @param stream, a pointer to file to write in.
 * @return the value of the caracter and in case of failiure it returns EOF.
 */
int fputc(char c, FILE *stream) {
    return (write(getfd(stream), &c, 1) == 1? c : EOF);
}

/**
 * Print a line in the given stream.
 *
 * @param s, the string to be written.
 * @param stream, a pointer to the file to write in.
 * @return the number of characters printed and in case of failiure it returns EOF.
 */
int fputs(const char *s, FILE *stream) {

    if (s != NULL) {
        int len = strlen(s);
        int total;
        total = write(getfd(stream), s, len);
        total = total + (fputc('\n', stream) > 0);

        return (total == len + 1? len : EOF);
    }

    return EOF;
}

/**
 * Returns the file descriptor of a file
 *
 * @param stream, a pointer to the file to be evaluated.
 * @return the stream file descriptor.
 */
int getfd(FILE *stream) {
    return stream->fd;
}

/**
 *  Prints with format given the FILE and the va_list initiated.
 *
 *  The variables sent will be printed in order specifying the types with a %
 *  symbol in the format.
 *  %d or %i indicates an integer is expected.
 *  %s indicates a pointer to a char is expected.
 *  %c indicates a character is expected.
 *  %u indicates an unsigned int is expected.
 *  %% indicates a % should be printed.
 *
 *  @param stream, a pointer to a file indicating where to write.
 *  @param format, a constant pointer to a char indicating the format of the text to be printed.
 *  @param arg, the list of variable arguments.
 *  @return the number of characters printed or -1 in case of failiure.
 */
int vfprintf(FILE *stream, const char *format, va_list arg) {

    int i = 0;
    int symb = 0;
    int lastprint = 0;
    int plus = 0;
    char buffint[MAX_BUF];
    char *buffstring;
    int sizestring;

    while (format[i] != '\0') {
        if (format[i] == '%') {
            if (write(getfd(stream), format + lastprint, i - lastprint)
                 != i - lastprint) {
                return -1;
            }

            i++;
            symb++;
            switch (format[i]) {
                case 'd':
                case 'i':
                    sizestring = itoa(buffint,va_arg(arg,int));
                    plus += sizestring;
                    if (write(getfd(stream),buffint,sizestring) != sizestring) {
                        return -1;
                    }
                    symb++;
                    break;
                case 'u':
                    sizestring = utoa(buffint,va_arg(arg, unsigned int));
                    plus += sizestring;
                    if (write(getfd(stream),buffint,sizestring) != sizestring) {
                        return -1;
                    }
                    symb++;
                    break;
                case 'c':
                    if (fputc(va_arg(arg,int), stream) == EOF) {
                        return -1;
                    }
                    break;
                case 's':
                    buffstring = va_arg(arg,char *);
                    sizestring = strlen(buffstring);
                    plus += sizestring;
                    if (write(getfd(stream),buffstring,sizestring) != sizestring) {
                        return -1;
                    }
                    symb++;
                    break;
                case '%':
                    if(fputc('%', stream) == EOF) {
                        return -1;
                    }
                    break;
            }
            i++;
            lastprint = i;
        } else {
            i++;
        }
    }
    if(write(getfd(stream),format + lastprint, i - lastprint)
                 != (i - lastprint) ) {
        return -1;
    }
    return i - symb + plus;
}

/**
 *  Prints with format on stdout.
 *
 *  This function receives variable arguments, containing the variables to be printed.
 *  The variables sent will be printed in order specifying the types with a %
 *  symbol in the format.
 *  %d or %i indicates an integer is expected.
 *  %s indicates a pointer to a char is expected.
 *  %c indicates a character is expected.
 *  %u indicates an unsigned int is expected.
 *  %% indicates a % should be printed.
 *
 *  @param format, a constant pointer to a char, containing the format to be printed.
 *  @return the number of characters printed or -1 in case of failiure.
 */
int printf(const char *format, ...) {

    va_list ap;
    va_start(ap, format);
    return vfprintf(stdout, format, ap);
}

/**
 * Prints with format given an output.
 *
 * This function receives variable arguments, containing the variables to be printed.
 *  The variables sent will be printed in order specifying the types with a %
 *  symbol in the format.
 *  %d or %i indicates an integer is expected.
 *  %s indicates a pointer to a char is expected.
 *  %c indicates a character is expected.
 *  %u indicates an unsigned int is expected.
 *  %% indicates a % should be printed.
 *
 * @param stream, a pointer to the file to be written.
 * @param format, a constant pointer to a char, containing the format to be printed.
 * @return the number of characters printed or -1 in case of failiure.
 */
int fprintf(FILE *stream, const char *format, ...) {

    va_list ap;
    va_start(ap, format);
    return vfprintf(stream, format, ap);
}

/**
 *  Prints with a format given with the variable list ap initialized.
 *
 *  The variables sent will be printed in order specifying the types with a %
 *  symbol in the format.
 *  %d or %i indicates an integer is expected.
 *  %s indicates a pointer to a char is expected.
 *  %c indicates a character is expected.
 *  %u indicates an unsigned int is expected.
 *  %% indicates a % should be printed.
 *
 *  @param format, a constant pointer to a char, containing the format to be printed.
 *  @param arg, a list of the variable arguments.
 *  @return the number of characters printed or -1 in case of failiure.
 */
int vprintf(const char *format, va_list arg) {

    return vfprintf(stdout, format, arg);
}

/**
 * Calls the system so it can write on the correct file.
 *
 * @param fd, the file descritor to be written.
 * @param cs, the string to be written.
 * @param n, the amount of characters to be written.
 * @return the number of characters written.
 */
size_t write(int fd, const char *cs, size_t n){
    return system_call(_SYS_WRITE, fd, (int) cs, n);
}

/**
 * Calls the system so it can read on the correct file.
 *
 * @param fd, the file descriptor to be read.
 * @param buf, this string will containg what it's read.
 * @param n, the amount of characters to be read.
 * @return the amount of characters read.
 */
size_t read(int fd, void *buf, size_t n) {
    return system_call(_SYS_READ, fd, (int)buf, n);
}

/**
 * Calls the system to do driver dependent operations.
 *
 * @param fd, the file descriptor of the file to be manipulated.
 * @parem cmd, the command to be executed.
 * @param argp, the arguments of the command.
 */
size_t ioctl(int fd, int cmd, void *argp) {
    return system_call(_SYS_IOCTL, fd, cmd, (int)argp);
}

/**
 * Returns the next character of stream.
 *
 * @param stream, a pointer to the FILE to be read.
 * @return returns the character read or EOF if fails. */
int fgetc(FILE *stream) {
    char c;
    if (stream->flag) {
        stream->flag = 0;
        return stream->unget;
    }
    return read(getfd(stream), &c, 1) == 1 ? c : EOF;
}

/**
 * Deals with formatted input conversion given a stream and a list of arguments.
 *
 * The variables sent will be scaned in order specifying the types with a %
 * symbol in the format.
 * %d or %i indicates an integer is expected.
 * %s indicates a pointer to a char is expected.
 * %c indicates a character is expected.
 * %u indicates an unsigned int is expected.
 * %% indicates a % should be taken in account.
 *
 * @param stream, a pointer to the file to be read.
 * @param format, a constant pointer to a char indicating what is about to be read.
 * @param arg, a list of variable arguments.
 * @return the number of converted variables.
 */
int vfscanf(FILE *stream, const char *format, va_list arg) {

    int i = 0;
    int j;
    int converted = 0;
    char buff[MAX_BUF];
    char cur;
    char *tempstring;

    while (format[i] != '\0') {

        if (isspace(format[i])) {
            cur = fgetc(stream);
            while (isspace(cur)) {
                cur = fgetc(stream);
            }
            ungetc(cur, stream);
        } else {
            if (format[i] != '%') {
                cur = fgetc(stream);
                while (isspace(cur) && cur != '\n') {
                    cur = fgetc(stream);
                }
                if (format[i] != cur) {
                    ungetc(cur, stream);
                    return converted;
                }
            } else {
                i++;
                switch (format[i]) {
                    case '%':
                        cur = fgetc(stream);
                        while (isspace(cur) && cur != '\n') {
                            cur = fgetc(stream);
                        }
                        if (cur != '%') {
                            ungetc(cur, stream);
                            return converted;
                        }
                        break;
                    case 'c':
                        cur = fgetc(stream);
                        while (isspace(cur) && cur != '\n') {
                            cur = fgetc(stream);
                        }
                        if (cur == EOF || cur == '\n') {
                            ungetc(cur,stream);
                            return converted;
                        }
                        *(va_arg(arg, char *)) = cur;
                        converted++;
                        break;
                    case 's':
                        tempstring = va_arg(arg, char *);
                        cur = fgetc(stream);
                        while (isspace(cur) && cur != '\n') {
                            cur = fgetc(stream);
                        }

                        if ( cur == EOF) {
                            return converted;
                        }
                        j = 0;
                        while (!isspace(cur) && cur != EOF && cur != '\n') {
                            tempstring[j] = cur;
                            cur = fgetc(stream);
                            j++;
                        }
                        ungetc(cur,stream);
                        tempstring[j] = '\0';
                        converted++;
                        break;
                    case 'i':
                    case 'd':
                        cur = fgetc(stream);
                        while (isspace(cur) && cur != '\n'){
                            cur = fgetc(stream);
                        }
                        if (!isdigit(cur) && cur != '-') {
                            ungetc(cur, stream);
                            return converted;
                        }
                        j = 0;
                        if (cur == '-') {
                            buff[j] = '-';
                            j++;
                            cur = fgetc(stream);
                            if(!isdigit(cur)){
                                ungetc(cur, stream);
                                return converted;
                            }
                        }
                        buff[j] = cur;
                        j++;
                        cur = fgetc(stream);
                        while(isdigit(cur)) {
                            buff[j] = cur;
                            j++;
                            cur = fgetc(stream);
                        }

                        ungetc(cur,stream);
                        buff[j] = '\0';
                        *(va_arg(arg, int *)) = atoi(buff);
                        converted ++;

                        break;
                    case 'u':
                        cur = fgetc(stream);
                        while (isspace(cur) && cur != '\n'){
                            cur = fgetc(stream);
                        }
                        if (!isdigit(cur)) {
                            ungetc(cur, stream);
                            return converted;
                        }
                        j = 0;
                        buff[j] = cur;
                        j++;
                        cur = fgetc(stream);
                        while(isdigit(cur)) {
                            buff[j] = cur;
                            j++;
                            cur = fgetc(stream);
                        }

                        ungetc(cur,stream);
                        buff[j] = '\0';
                        *(va_arg(arg, int *)) = atou(buff);
                        converted ++;

                        break;
                }
            }
        }
        i++;
    }
    return converted;
}

/**
 *  Prepares the character c to be read the next time you access stream.
 *
 *  @param c, the character to be unread.
 *  @param stream, a pointer to the file to be used.
 *  @return the same character c or EOF in case of failiure.
 */
int ungetc(int c, FILE *stream) {
    if (c == EOF || stream->flag) {
        return EOF;
    }
    stream->flag = 1;
    stream->unget = c;
    return c;
}

/**
 * Deals with a formatted input.
 *
 * This function receives variable arguments, containing the variables to be read.
 * The variables sent will be scaned in order specifying the types with a %
 * symbol in the format.
 * %d or %i indicates an integer is expected.
 * %s indicates a pointer to a char is expected.
 * %c indicates a character is expected.
 * %u indicates an unsigned int is expected.
 * %% indicates a % should be taken in account.
 *
 * @param format, a constant pointer to a char indicating what is about to be read.
 * @return the number of converted variables.
 */
int scanf(const char *format, ...) {

    va_list ap;
    va_start(ap, format);
    return vfscanf(stdin, format, ap);
}

/**
 *  Deals with a formatted input given the initialized variables list.
 *
 *  The variables sent will be scaned in order specifying the types with a %
 *  symbol in the format.
 *  %d or %i indicates an integer is expected.
 *  %s indicates a pointer to a char is expected.
 *  %c indicates a character is expected.
 *  %u indicates an unsigned int is expected.
 *  %% indicates a % should be taken in account.
 *
 *  @param format, a constant pointer to a char indicating what is about to be read.
 *  @param arg, the list of variable arguments.
 *  @return the number of converted variables
 */
int vscanf(const char *format, va_list arg) {
    return vfscanf(stdin, format, arg);
}

/**
 * Deals with a formatted input given a stream.
 *
 * The variables sent will be scaned in order specifying the types with a %
 * symbol in the format.
 * %d or %i indicates an integer is expected.
 * %s indicates a pointer to a char is expected.
 * %c indicates a character is expected.
 * %u indicates an unsigned int is expected.
 * %% indicates a % should be taken in account.
 *
 * This function receives variable arguments, containing the variables to be read.
 * @param stream, a pointer to the file to read.
 * @param format, a constant pointer to a char indicating what is about to be read.
 */
int fscanf(FILE *stream, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    return vfscanf(stream, format, ap);
}

/**
 * Closes a file already opened by the process.
 *
 * @param fd, the file descriptor to be closed.
 * @return 0 if success, -1 if error.
 */
int close(int fd) {
    return system_call(_SYS_CLOSE, fd, 0, 0);
}

/**
 * Opens a file.
 *
 * @param filename, the string where the file is located.
 * @param flags, the access mode.
 * @param mode, in case O_CREAT is specified in the flags, a third parameter,
 *              mode indicates the permissions of the new file.
 * @return a number representing the file descriptor on succes, -1 on error.
 */
int open(const char* filename, int flags, ...) {
    int mode;
    if (flags & O_CREAT) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, int);
        va_end(ap);
    } else {
        mode = 0;
    }
    return system_call(_SYS_OPEN, (int)filename, flags, mode);
}

/*
 * Opens a file and returns FILE structure to be used by other funciotns of the stdio lib.
 *  NOTE: currently we only handle the cases for text files, the 'b' character in the mode
 *  string is not supported.
 *
 *  @param filename the string with the file's path.
 *  @param mode a string containing a file access modes.
 *
 *  @return a FILE structure representing the file opened on success, NULL on error.
 */
FILE* fopen(const char* filename, const char* mode) {
    int fd;
    disableInterrupts();
    FILE *fp = kalloc(sizeof(FILE));
    enableInterrupts();

    int  hasR = strchr(mode, 'r') != NULL;
    int hasW = strchr(mode, 'w') != NULL;
    int hasA = strchr(mode, 'a') != NULL;
    int hasPlus = strchr(mode, '+') != NULL;

    if ( !hasR && !hasW && !hasA){
        return NULL;
    }

    if (hasR) {
        if (hasPlus) {
            fd = open(filename, O_RDWR, 0666);
        } else {
            fd = open(filename, O_RDONLY, 0666);
        }
    } else if (hasW) {
        // the O_TRUNC flag does nothing now, it is left for future implementations, in
        // which the unlink should be blow away
        unlink(filename);
        if (hasPlus) {
            fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
        } else {
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        }
    } else if (hasA) {
        if (hasPlus) {
            fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
        } else {
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
        }
    }

    if (fd == -1) {        /* couldn't access name */
        return NULL;
    }

    fp->fd = fd;
    fp->flag = 0;
    fp->unget = 0;
    return fp;
}

int fclose(FILE* stream) {
    int fd = stream->fd;
    free(stream);
    return close(fd);
}


/**
 * Creates a new directory.
 *
 * @param path, the path of the directory to be created.
 * @param mode, the permissions of the directory.
 * @return 0 if success, other if error.
 */
int mkdir(const char* path, int mode) {
    return system_call(_SYS_MKDIR, (int) path, mode, 0);
}

/**
 * Removes a directory if it is empty.
 *
 * @param path, the path of the directory to be erased.
 * @return 0 if success, other if error.
 */
int rmdir(const char* path) {
    return system_call(_SYS_RMDIR, (int) path, 0, 0);
}

/**
 * Remove a hard link from the file
 *
 * @param path, the path of the file to be unlinked.
 * @return 0 if success, other if error
 */
int unlink(const char* path) {
    return system_call(_SYS_UNLINK, (int) path, 0, 0);
}

/**
 * Renames a file.
 * @param from, the original path of the file.
 * @param to, the new path of the file.
 * @return 0 if success, other if error.
 */
int rename(const char* from, const char* to) {
    return system_call(_SYS_RENAME, (int) from, (int) to, 0);
}

/**
 * Reads an entry from a directory.
 *
 * @param fd, the file descriptor of the directory to be read.
 * @param entry, the output of the entry to be read.
 * @param hidden, should value 1 if hidden files are also wanted.
 * @return, 1 if something was read, 0 if not.
 */
int readdir(int fd, struct fs_DirectoryEntry* entry, int hidden) {
    return system_call(_SYS_READDIR, fd, (int) entry, hidden);
}

/**
 * Changes the cwd.
 *
 * @param path, the path of the new cwd.
 * @return 0 if success, other if error.
 */
int chdir(const char* path) {
    return system_call(_SYS_CHDIR, (int) path, 0, 0);
}

/**
 * Get the cwd.
 *
 * @param path, the string which will contain the cwd.
 * @param len, the maximum lenght of the string accepted.
 * @return 0 if success, other if error.
 */
int getcwd(char* path, size_t len) {
    return system_call(_SYS_GETCWD, (int) path, (int) len, 0);
}

/**
 * Create a symbolic link.
 *
 * @param path, the symbolic link to be created.
 * @param target, the target which will be referenced by path.
 * @return 0 if success, other if error.
 */
int symlink(const char* path, const char* target) {
    return system_call(_SYS_SYMLINK, (int) path, (int) target, 0);
}

/**
 * Create a named pipe.
 *
 * @param path, the path of the pipe to be created.
 * @return 0 if success, other if error
 */
int mkfifo(const char* path) {
    return system_call(_SYS_MKFIFO, (int) path, 0, 0);
}

