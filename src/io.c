#include "../include/io.h"

inline unsigned char inB(unsigned short port) {
    unsigned char ret;

    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

inline void outB(unsigned short port, unsigned char data) {
    asm volatile ("outb %0, %1" : "=a"(data) : "Nd"(port));
}
