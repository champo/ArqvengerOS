#include "library/ctype.h"

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
