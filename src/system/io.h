#ifndef _system_io_header
#define _system_io_header

unsigned char inB(unsigned short port);

void outB(unsigned short port, unsigned char data);

unsigned int inD(unsigned short port);

void outD(unsigned short port, unsigned int data);

#endif
