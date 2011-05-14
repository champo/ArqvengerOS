#ifndef _system_interrupt_handler_header_
#define _system_interrupt_handler_header_

#define     _SYS_READ       3
#define     _SYS_WRITE      4
#define     _SYS_IOCTL      54

void setInterruptHandlerTable(void);
#endif
