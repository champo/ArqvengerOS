#ifndef _system_interrupt_header_
#define _system_interrupt_header_

void setupIDT(void);

unsigned int idt_page_address(void);

void interrupt_disable_disk(void);

void interrupt_enable_disk(void);

#endif
