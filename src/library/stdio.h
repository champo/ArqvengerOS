#ifndef __stdio_header__

#define __stdio_header__

#define putchar(x) fputc((x),stdout)
#define putc(x, stream) fputc((x), (stream))
#define puts(s) fputs((s),stdout)
#define EOF -1
#define NULL (void *)0
#define getc(stream) fputc((stream))
#define getchar() fgetc(stdin)

#include "library/stdarg.h"

typedef struct {
    int fd;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fputc(char c, FILE *stream);
int fputs(const char *s, FILE *stream);
int vfprintf(FILE *stream, const char *format, va_list arg);
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vprintf(const char *format, va_list arg);
int fgetc(FILE *stream);

#endif
