/*********************************************
kasm.h

************************************************/
#ifndef _kasm_header_
#define _kasm_header_

#include "../include/interrupt.h"

unsigned int _read_msw();

void _lidt(InterruptDescriptorTableRegister* idtr);

void _cli(void);
void _sti(void);

void _int08Handler();
void _int09Handler();
#endif
