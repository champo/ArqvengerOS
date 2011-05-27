#include "library/ctype.h"

/**
 * Checks if the character represents a digit.
 */
int isdigit(int c) {
    return c >= '0' && c <= '9';
}

/**
 * Checks if the character represents a space.
 */
int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

/**
 * Checks if the character is an upper case letter.
 */
int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

/**
 * Checks if the character is a lower case letter.
 */
int islower(int c) {
    return c >= 'a' && c <= 'z';
}

/** 
 * Checks if the character is a letter.
 */
int isalpha(int c) {
    return isupper(c) || islower(c);
}

/**
 * Checks if the character is an alphanumeric symbol.
 */
int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}
