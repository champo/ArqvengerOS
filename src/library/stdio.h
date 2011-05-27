#ifndef __stdio_header__

#define __stdio_header__

#include "type.h"

#define putchar(x) fputc((x),stdout)
#define putc(x, stream) fputc((x), (stream))
#define puts(s) fputs((s),stdout)
#define EOF -1
#define getc(stream) fputc((stream))
#define getchar() fgetc(stdin)

#include "library/stdarg.h"

typedef struct {
    int fd;
    int flag;
    int unget;
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
size_t ioctl(FILE *stream, int cmd, void *argp);
int ungetc(int c, FILE *stream);
int vfscanf(FILE *stream, const char *format, va_list arg);
int scanf(const char *format, ...);
int getfd(FILE *stream);
int vscanf(const char *format, va_list arg);
int fscanf(FILE *stream, const char *format, ...);

#endif
