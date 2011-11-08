#ifndef __stdio_header__

#define __stdio_header__

#include "type.h"
#include "system/fs/direntry.h"
#include "constants.h"

#define putchar(x) fputc((x),stdout)
#define putc(x, stream) fputc((x), (stream))
#define puts(s) fputs((s),stdout)
#define EOF -1
#define getc(stream) fputc((stream))
#define getchar() fgetc(stdin)

#include "system/stat.h"
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
size_t ioctl(int fd, int cmd, void *argp);
int ungetc(int c, FILE *stream);
int vfscanf(FILE *stream, const char *format, va_list arg);
int scanf(const char *format, ...);
int getfd(FILE *stream);
int vscanf(const char *format, va_list arg);
int fscanf(FILE *stream, const char *format, ...);
int close(int fd);
int open(const char* filename, int flags, ...);

FILE * fopen(const char* filename, const char* mode);
int fclose(FILE* stream);

size_t write(int fd, const char *cs, size_t n);

size_t read(int fd, void *buf, size_t n);

int mkdir(const char* path, int mode);

int rmdir(const char* path);

int unlink(const char* path);

int rename(const char* from, const char* to);

int readdir(int fd, struct fs_DirectoryEntry* entry, int hidden);

int chdir(const char* path);

int getcwd(char* path, size_t len);

int symlink(const char* path, const char* target);

int mkfifo(const char* path);

int chmod(int mode, char* file);

int stat(const char* path, struct stat* data);

char* path_directory(const char* path);

char* path_file(const char* path);

char* join_paths(const char* cwd, const char* path);

void loglevel(int level);

#endif
