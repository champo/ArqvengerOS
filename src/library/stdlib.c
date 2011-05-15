#include "library/ctype.h"
#include "library/string.h"
#include "library/stdlib.h"

static size_t randSeed;

/**
 * Converts a string to its equivalent int
 */
int atoi(const char *s) {

    int i = 0;
    int negative = 1;
    int ans = 0;

    while(isspace(s[i])) {
        i++;
    }

    if (s[i] == '-'){
        negative = -1;
        i++;
        while(isspace(s[i])) {
            i++;
        }
    }

    while(isdigit(s[i])){
        ans *= 10;
        ans = ans + s[i] - '0';
        i++;
    }

    return (negative * ans);
}

int itoa(char *s, int n){

    int i = 0;
    int aux;
    int neg = 0;

    if (n < 0) {
        neg = 1;
        n = n * -1;
    }

    do {
        aux = n%10;
        s[i] = aux + '0';
        n = n / 10;
        i++;
    } while(n > 0);

    if (neg) {
        s[i] = '-';
        i++;
    }

    s[i] = '\0';

    reverse(s);

    return i;
}

int rand(void) {

    randSeed = (size_t) ((1103515245UL * (unsigned long) randSeed + 12345UL) % (1<<30));
    return (int) (randSeed >> 2);
}

void srand(unsigned int seed) {
    randSeed = seed;
}

