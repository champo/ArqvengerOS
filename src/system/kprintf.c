#include "system/kprintf.h"
#include "library/stdarg.h"
#include "drivers/tty/tty.h"
#include "library/stdlib.h"
#include "library/string.h"

#ifndef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL LOG_DEBUG
#endif

static int logLevel = DEFAULT_LOG_LEVEL;

static void putc(int terminal, char c) {
    tty_write_to_terminal(terminal, &c, 1);
}

int tkprintf(int terminal, const char* format, va_list arg);

/**
 *  Prints with format to the kernel given terminal.
 *
 *  The variables sent will be printed in order specifying the types with a %
 *  symbol in the format.
 *  %d or %i indicates an integer is expected.
 *  %s indicates a pointer to a char is expected.
 *  %c indicates a character is expected.
 *  %u indicates an unsigned int is expected.
 *  %% indicates a % should be printed.
 *
 *  @param format, a constant pointer to a char indicating the format of the text to be printed.
 *
 *  @return the number of characters printed or -1 in case of failiure.
 */
int tkprintf(int terminal, const char *format, va_list arg) {

    int i = 0;
    int symb = 0;
    int lastprint = 0;
    int plus = 0;
    char buffint[15];
    char *buffstring;
    int sizestring;

    while (format[i] != '\0') {
        if (format[i] == '%') {
            tty_write_to_terminal(terminal, format + lastprint, i - lastprint);

            i++;
            symb++;
            switch (format[i]) {
                case 'd':
                case 'i':
                    sizestring = itoa(buffint, va_arg(arg, int));
                    plus += sizestring;
                    tty_write_to_terminal(terminal, buffint, sizestring);
                    symb++;
                    break;
                case 'u':
                    sizestring = utoa(buffint, va_arg(arg, unsigned int));
                    plus += sizestring;
                    tty_write_to_terminal(terminal, buffint, sizestring);
                    symb++;
                    break;
                case 'c':
                    putc(terminal, va_arg(arg, int));
                    break;
                case 's':
                    buffstring = va_arg(arg, char *);
                    sizestring = strlen(buffstring);
                    plus += sizestring;
                    tty_write_to_terminal(terminal, buffstring, sizestring);
                    symb++;
                    break;
                case '%':
                    putc(terminal, '%');
                    break;
            }
            i++;
            lastprint = i;
        } else {
            i++;
        }
    }

    tty_write_to_terminal(terminal, format + lastprint, i - lastprint);

    return i - symb + plus;
}

int kprintf(const char* format, ...) {

    va_list arg;
    va_start(arg, format);

    int res = tkprintf(NO_TERMINAL, format, arg);

    va_end(arg);
    return res;
}

int log_debug(const char* format, ...) {

    if (logLevel < LOG_DEBUG) {
        return 0;
    }

    va_list arg;
    va_start(arg, format);

    int res = tkprintf(4, format, arg);

    va_end(arg);
    return res;
}

int log_info(const char* format, ...) {

    if (logLevel < LOG_INFO) {
        return 0;
    }

    va_list arg;
    va_start(arg, format);

    int res = tkprintf(4, format, arg);

    va_end(arg);
    return res;
}

int log_error(const char* format, ...) {

    if (logLevel < LOG_ERROR) {
        return 0;
    }

    va_list arg;
    va_start(arg, format);

    int res = tkprintf(4, format, arg);

    va_end(arg);
    return res;
}

void set_log_level(int level) {
    logLevel = level;
}

