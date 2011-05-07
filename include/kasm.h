/*********************************************
kasm.h

************************************************/

#include "defs.h"


unsigned int _read_msw();

void _lidt(IDTR *idtr);

void _Cli(void);        /* Deshabilita interrupciones  */
void _Sti(void);	 /* Habilita interrupciones  */

void _int_08_hand();      /* Timer tick */
void _int09Handler();

void _debug (void);

