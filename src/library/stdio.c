#include "system/call.h"
#include "library/string.h"
#include "library/stdio.h"


/**
 * Insert a character into standard output
 */
int putc(char c, int fd) {
     return (write(fd, &c, 1) == 1? c : EOF);
}

/**
 * Print a line in the standard output
 */
int puts(const char *s) {
    int len = strlen(s);

    return ((write(1, s, len) + putchar('\n')) == len + 1? len : EOF);

}
