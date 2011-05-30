#include "system/io.h"


/**
 * Wrapper function of inline assembler instruction inb.
 *
 * @param port The number of the port to be accessed.
 *
 * @return The data read from the port.
 */
inline unsigned char inB(unsigned short port) {
    unsigned char ret;

    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * Wrapper function of inline assembler instruction outb.
 *
 * @param port The number of the port to be accessed.
 * @param data The data to be written to port.
 */
inline void outB(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}
