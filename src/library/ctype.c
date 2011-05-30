#include "library/ctype.h"

/**
 * Checks if the character represents a digit.
 *
 * @param c the character to be tested. 
 * @return 1 if the caracter is a digit, 0 if not.
 */
int isdigit(int c) {
    return c >= '0' && c <= '9';
}

/**
 * Checks if the character represents a space.
 * 
 * @param c the character to be tested. 
 * @return 1 if the caracter is a space, 0 if not.
 */
int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

/**
 * Checks if the character is an upper case letter.
 * 
 * @param c the character to be tested. 
 * @return 1 if the caracter is an uppercase letter, 0 if not.
 */
int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

/**
 * Checks if the character is a lower case letter.
 *
 * @param c the character to be tested. 
 * @return 1 if the caracter is a lowercase letter, 0 if not.
 */
int islower(int c) {
    return c >= 'a' && c <= 'z';
}

/** 
 * Checks if the character is a letter.
 *
 * @param c the character to be tested. 
 * @return 1 if the caracter is a letter, 0 if not.
 */
int isalpha(int c) {
    return isupper(c) || islower(c);
}

/**
 * Checks if the character is an alphanumeric symbol.
 *
 * @param c the character to be tested. 
 * @return 1 if the caracter is a digit or a letter, 0 if not.
 */
int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}
