#include "system/call.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "library/limits.h"
#include "library/stdarg.h"
#include "type.h"

FILE *stdout = {{1}};
FILE *stdin = {{0}};
FILE *stderr = {{2}};

size_t systemWrite(FILE *stream, const char *cs, size_t n);
size_t systemRead(FILE *stream, void *buf, size_t n);
int getfd(FILE *stream);

/**
 * Insert a character into standard output
 */
int fputc(char c, FILE *stream) {

    return (systemWrite(stream, &c, 1) == 1? c : EOF);
}

/**
 * Print a line in the standard output
 */
int fputs(const char *s, FILE *stream) {

    if(s != NULL){
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
 * Prints with format given the FILE and the va_list initiated
 */
int vfprintf(FILE *stream, const char *format, va_list arg) {
    
    int i = 0;
    int symb = 0;
    int lastprint = 0;
    int plus = 0;
    char buffint[MAX_BUF];
    char *buffstring;
    int sizestring;
    
    while(format[i] != '\0') {
        if(format[i] == '%') {
            if(systemWrite(stream,format + lastprint, i - lastprint)
                 != i - lastprint ) {
                return -1;
            }
            
            i++;
            symb++;
            switch (format[i]) {
                case 'd':
                case 'i':
                    sizestring = itoa(buffint,va_arg(arg,int));
                    plus += sizestring;
                    if(systemWrite(stream,buffint,sizestring) != sizestring) {
                        return -1;
                    }    
                    symb++;
                    break;
                case 'c':
                    if(fputc(va_arg(arg,int), stream) == EOF) {
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
                //TODO HACER LOS CASE NECESARIOS, ACORDARSE DE SUMAR PLUS
            }
            i++;
            lastprint = i;
        } else {    
            i++;
        }
    }
    if(systemWrite(stream,format + lastprint, i - lastprint)
                 != i - lastprint ) {
        return -1;
    }
    return i - symb + plus;
}

/**
 *  Prints with format on stdout
 */
int printf(const char *format, ...) {
    
    va_list ap;
    va_start(ap, format);
    return vfprintf(stdout, format, ap);
}

/**
 * Prints with format given an output
 */
int fprintf(FILE *stream, const char *format, ...) {
        
    va_list ap;
    va_start(ap, format);
    return vfprintf(stream, format, ap);
}

/** 
 * Prints with a format given with the variable list ap initialized
 */
int vprintf(const char *format, va_list arg) {
    
    return vfprintf(stdout, format, arg);
}

/**
 * Calls the system so it can write on the correct file
 */
size_t systemWrite(FILE *stream, const char *cs, size_t n){
    return write(getfd(stream), cs, n);
}

/**
 * Calls the system so it can read on the correct file
 */
size_t systemRead(FILE *stream, void *buf, size_t n) {
    return read(getfd(stream), buf, n);
}

/**
 * Returns the next character of stream
 */
int fgetc(FILE *stream) {
    char c;
    return systemRead(stream, &c, 1) ? c : EOF;
}
