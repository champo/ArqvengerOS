#include "system/kprintf.h"
#include "library/stdarg.h"
#include "drivers/tty/tty.h"
#include "library/stdlib.h"
#include "library/string.h"

static void putc(char c) {
    tty_write(&c, 1);
}

/**
 *  Prints with format to the kernel output.
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
int kprintf(const char *format, ...) {

    va_list arg;
    va_start(arg, format);

    int i = 0;
    int symb = 0;
    int lastprint = 0;
    int plus = 0;
    char buffint[15];
    char *buffstring;
    int sizestring;

    while (format[i] != '\0') {
        if (format[i] == '%') {
            tty_write(format + lastprint, i - lastprint);

            i++;
            symb++;
            switch (format[i]) {
                case 'd':
                case 'i':
                    sizestring = itoa(buffint, va_arg(arg, int));
                    plus += sizestring;
                    tty_write(buffint, sizestring);
                    symb++;
                    break;
                case 'u':
                    sizestring = utoa(buffint, va_arg(arg, unsigned int));
                    plus += sizestring;
                    tty_write(buffint, sizestring);
                    symb++;
                    break;
                case 'c':
                    putc(va_arg(arg, int));
                    break;
                case 's':
                    buffstring = va_arg(arg, char *);
                    sizestring = strlen(buffstring);
                    plus += sizestring;
                    tty_write(buffstring, sizestring);
                    symb++;
                    break;
                case '%':
                    putc('%');
                    break;
            }
            i++;
            lastprint = i;
        } else {
            i++;
        }
    }

    tty_write(format + lastprint, i - lastprint);

    return i - symb + plus;
}
