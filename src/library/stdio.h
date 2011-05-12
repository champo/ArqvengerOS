#ifndef __stdio_header__

#define __stdio_header__

#define putchar(x) putc((x),1)
#define EOF -1
#define NULL (void *)0

int putc(char c, int fd);
int puts(const char *s);

#endif
