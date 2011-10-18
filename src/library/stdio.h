#ifndef __stdio_header__

#define __stdio_header__

#include "type.h"

#define putchar(x) fputc((x),stdout)
#define putc(x, stream) fputc((x), (stream))
#define puts(s) fputs((s),stdout)
#define EOF -1
#define getc(stream) fputc((stream))
#define getchar() fgetc(stdin)

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

/* Oflag values for open().  POSIX Table 6-4. */
#define O_CREAT        00100    /* creat file if it doesn't exist */
#define O_EXCL         00200    /* exclusive use flag */
#define O_NOCTTY       00400    /* do not assign a controlling terminal */
#define O_TRUNC        01000    /* truncate flag */
 
/* File status flags for open() and fcntl().  POSIX Table 6-5. */
#define O_APPEND       02000    /* set append mode */
#define O_NONBLOCK     04000    /* no delay */
 
/* File access modes for open() and fcntl().  POSIX Table 6-6. */
#define O_RDONLY           0    /* open(name, O_RDONLY) opens read only */
#define O_WRONLY           1    /* open(name, O_WRONLY) opens write only */
#define O_RDWR             2    /* open(name, O_RDWR) opens read/write */

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
int close(FILE *stream);
int open(char* filename, int flags, ...);

#endif
