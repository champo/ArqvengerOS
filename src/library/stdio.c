#include "system/call.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/limits.h"
#include "library/stdarg.h"
#include "type.h"
#include "system/call/codes.h"
#include "library/ctype.h"

FILE *stdout;
FILE *stdin;
FILE *stderr;

extern size_t systemCall(int eax, int ebx, int ecx, int edx);

size_t systemWrite(FILE *stream, const char *cs, size_t n);

size_t systemRead(FILE *stream, void *buf, size_t n);

/**
 * Insert a character into the given stream.
 *
 * It returns the value of the caracter and in case of failiure it returns EOF.
 */
int fputc(char c, FILE *stream) {
    return (systemWrite(stream, &c, 1) == 1? c : EOF);
}

/**
 * Print a line in the given stream.
 *
 * It returns the number of characters printed and in case of failiure it returns EOF.
 */
int fputs(const char *s, FILE *stream) {

    if (s != NULL) {
        int len = strlen(s);
        int total;
        total = systemWrite(stream, s, len);
        total = total + (fputc('\n', stream) > 0);

        return (total == len + 1? len : EOF);
    }

    return EOF;
}

/**
 * Returns the file descriptor of a file
 */
int getfd(FILE *stream) {
    return stream->fd;
}

/**
 * Prints with format given the FILE and the va_list initiated.
 *
 *  It returns the number of characters printed or -1 in case of failiure.
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
            if (systemWrite(stream, format + lastprint, i - lastprint)
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
                    if (systemWrite(stream,buffint,sizestring) != sizestring) {
                        return -1;
                    }
                    symb++;
                    break;
                case 'u':
                    sizestring = utoa(buffint,va_arg(arg, unsigned int));
                    plus += sizestring;
                    if (systemWrite(stream,buffint,sizestring) != sizestring) {
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
                    if (systemWrite(stream,buffstring,sizestring) != sizestring) {
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
    if(systemWrite(stream,format + lastprint, i - lastprint)
                 != (i - lastprint) ) {
        return -1;
    }
    return i - symb + plus;
}

/**
 *  Prints with format on stdout.
 *
 *  Returns the same as vfprintf.
 */
int printf(const char *format, ...) {

    va_list ap;
    va_start(ap, format);
    return vfprintf(stdout, format, ap);
}

/**
 * Prints with format given an output.
 *
 * Returns the same as vfprintf.
 */
int fprintf(FILE *stream, const char *format, ...) {

    va_list ap;
    va_start(ap, format);
    return vfprintf(stream, format, ap);
}

/**
 * Prints with a format given with the variable list ap initialized.
 *
 * Returns the same as vfprintf.
 */
int vprintf(const char *format, va_list arg) {

    return vfprintf(stdout, format, arg);
}

/**
 * Calls the system so it can write on the correct file
 */
size_t systemWrite(FILE *stream, const char *cs, size_t n){
    return systemCall(_SYS_WRITE, getfd(stream), (int) cs, n);
}

/**
 * Calls the system so it can read on the correct file
 */
size_t systemRead(FILE *stream, void *buf, size_t n) {
    return systemCall(_SYS_READ, getfd(stream), (int)buf, n);
}
/**
 *  Calls the system to do driver dependent operations
 */
size_t ioctl(FILE *stream, int cmd, void *argp) {
    return systemCall(_SYS_IOCTL, getfd(stream), cmd, (int)argp);
}

/**
 * Returns the next character of stream.
 *
 * In case of failiure it returns EOF.
 */
int fgetc(FILE *stream) {
    char c;
    if (stream->flag) {
        stream->flag = 0;
        return stream->unget;
    }
    return systemRead(stream, &c, 1) ? c : EOF;
}

/**
 * Deals with formatted input conversion given a stream and a list of arguments.
 *
 * It returns the number of converted variables.
 */
int vfscanf(FILE *stream, const char *format, va_list arg) {

    int i = 0;
    int j;
    int converted = 0;
    char buff[MAX_BUF];
    char cur;
    char *tempstring;

    while (format[i] != '\0') {
        if (!isspace(format[i])) {
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
 *  Returns the same character or EOF in case of failiure.
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
 * Returns the same as vfscanf.
 */
int scanf(const char *format, ...) {

    va_list ap;
    va_start(ap, format);
    return vfscanf(stdin, format, ap);
}

/**
 *  Deals with a formatted input given the initialized variables list.
 *
 *  Returns the same as vfscanf.
 */
int vscanf(const char *format, va_list arg) {
    return vfscanf(stdin, format, arg);
}

/**
 * Deals with a formatted input given a stream.
 *
 * Returns the same as vfscanf.
 */
int fscanf(FILE *stream, const char *format, ...){
    va_list ap;
    va_start(ap, format);
    return vfscanf(stream, format, ap);
}
