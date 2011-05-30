#include "library/ctype.h"
#include "library/string.h"
#include "library/stdlib.h"

static size_t randSeed;

/**
 * Converts a string to its equivalent int.
 *
 * @param s, a constant pointer to a char indicating the number to be analyzed.
 * @return the number analyzed.
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

/**
 * Converts an int to its equivalent string.
 *
 * @param s, the string where the number will be written.
 * @param n, the integer to be analyzed.
 * @return the lenght of the string.
 */
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

/**
 * Returns a random number.
 */
int rand(void) {

    randSeed = (size_t) ((1103515245UL * (unsigned long) randSeed + 12345UL) % (1<<30));
    return (int) (randSeed >> 2);
}

/**
 * Sets the seed for the random number generation.
 */
void srand(unsigned int seed) {
    randSeed = seed;
}

/**
 * Converts a string to its equivalent unsigned int.
 *
 * @param s, a constant string containing the number to be analyzed.
 * @return the number analyzed.
 */
unsigned int atou(const char *s) {

    int i = 0;
    unsigned int ans = 0;

    while(isspace(s[i])) {
        i++;
    }


    while(isdigit(s[i])){
        ans *= 10;
        ans = ans + s[i] - '0';
        i++;
    }

    return ans;
}

/**
 * Converts an unsigned int to its equivalent string.
 *
 * @param s, the string where the number will be written.
 * @param n, the number to be analyzed.
 * @returns the lenght of the string.
 */
int utoa(char *s, unsigned int n){

    int i = 0;
    int aux;


    do {
        aux = n%10;
        s[i] = aux + '0';
        n = n / 10;
        i++;
    } while(n > 0);


    s[i] = '\0';

    reverse(s);

    return i;
}
