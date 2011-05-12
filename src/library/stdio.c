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

    if(s != NULL){
        int len = strlen(s);
        int total;
        total = write(1, s, len);
        total = total + (putchar('\n') > 0);

        return (total == len + 1? len : EOF);
    }
    return EOF;
}
