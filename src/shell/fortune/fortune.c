#include "shell/fortune/fortune.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "mcurses/mcurses.h"
#include "library/time.h"
#define CANT_FORTUNES 19

static const char *phrases[] = { "The Arqvengers manifest: \n\nYet to be done..." ,
                    "This was a triumph. \nI'm making a note here: Huge Success.\nIt's hard to overstate my satisfaction." ,
                    "How to survive a giant assigment in college: \n\nJust put Queen's Under pressure in an infinite loop." ,
                    "Never get a roommate called Tebex. They never clean." ,
                    "ALVANATOR NEEDS FOOD." ,
                    "Never get a roommate called Alvanator. They just never get any showers." ,
                    "Sale Thor?" ,
                    "Ya viste Game of Thrones no? \nNO LO VISTE?!\nQUE HACES EN ESTA SHELL?!" ,
                    "Uno de los mejores momentos en la vida es cuando encontras un carribar." ,
                    "La batata macabra arranca aca! \nMarta jamas agarraba la bata.\nLa bata faltaba!\nY Marta andaba. \nNAAAAAH" ,
                    "Okay, you know what the three of us are? We're a tripod.\nA tripod? \nYes, a tripod. Which means that if you knock out one of our legs, WE-ALL-FALL!",
                    "Always know if the juice is worth the squeeze.",
                    "It's a trap!!!",
                    "Cuanto es 2^32?Mucho, no?",
                    "When you want something in life, you just gotta reach out and grab it. - Into the Wild.",
                    "Happiness only real when shared. - Into the Wild.",
                    "It's only after we've lost everything that we're free to do anything. - Fight Club.",
                    "Don't give up!Never give up!Never back down!!! - Never Back Down.",
                    "No matter what happens, control the outcome. It's on you. - Never Back Down.",
                    "It's not a bug, it's a feature"
        };

/**
 * Executes the fortune command meaning the user will receive an awesome phrase.
 */
void fortune(char* unused) {
    srand(time(NULL));
    printf("%s\n", phrases[rand() % CANT_FORTUNES]);
}

/**
 * Manual page for the fortune command.
 */
void manFortune(void) {
    setBold(1);
    printf("Usage:\n\t fortune \n");
    setBold(0);
}
