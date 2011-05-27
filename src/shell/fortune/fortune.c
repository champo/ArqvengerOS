#include "shell/echo/echo.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "mcurses/mcurses.h"
#include "library/time.h"
#define CANT_FORTUNES 10

void fortune(void) {
    char *phrases[] = { "The Arqvengers manifest: \n\nYet to be done..." , 
                        "This was a triumph. \nI'm making a note here: Huge Success.\nIt's hard to overstate my satisfaction." ,
                        "How to survive a giant assigment in college: \n\nJust put Queen's Under pressure in an infinite loop" ,                  
                        "Never get a roommate called Tebex. They never clean" , 
                        "ALVANATOR NEEDS FOOD" ,
                        "Never get a roommate called Alvanator. They just never get any showers" ,
                        "Sale Thor?" ,
                        "Ya viste Game of Thrones no? \nNO LO VISTE?!\nQUE HACES EN ESTA SHELL?!" ,
                        "Uno de los mejores momentos en la vida es cuando encontras un carribar" ,
                        "La batata macabra arranca aca! \nMarta jamas agarraba la bata.\nLa bata faltaba!\nY Marta andaba. \nNAAAAAH" ,
                         };
    srand(time(NULL));
    printf("%s\n", phrases[rand() % CANT_FORTUNES]);
}

void manFortune(void) {
    setBold(1);
    printf("Usage:\n\t fortune \n");
    setBold(0);
}
