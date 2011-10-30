#include "type.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "limits.h"
#include "string.h"


/**
 * Returns the lenght of a string withouth counting the final \0.
 *
 * @param s, the constant string to be evaluated.
 * @return the length of the string.
 */
size_t strlen(const char *s) {

    int i = 0;

    while (*(s+i) != '\0') {
        i++;
    }
    return i;
}

/**
 * Copies the string ct to string s.
 *
 * @param s, the destiny string of the copy.
 * @param ct, a constant string, the origin of the copy.
 * @return the string s.
 */
char *strcpy(char *s, const char *ct) {
    int i = 0;

    while (ct[i] != '\0') {
        s[i] = ct[i];
        i++;
    }

    s[i] = '\0';
    return s;
}

/**
 * Copies a maximum of n characters from ct to n.
 *
 * @param s, the destiny string of the copy.
 * @param ct, a constant string, the origin of the copy.
 * @param n, the number of characters to be copied.
 * @return the string s.
 */
char *strncpy(char *s, const char *ct, size_t n) {

    int i;

    for (i = 0; i < n && ct[i] != '\0'; i++) {
        s[i] = ct[i];
    }
    s[i] = '\0';

    return s;
}

/**
 * Concatenates string s with string ct.
 *
 * @param s, the string to be operated.
 * @param ct, a constant string to be concatenated in s.
 * @return the string s.
 */
char *strcat(char *s, const char *ct) {

    int i = 0;
    int j = 0;

    while (s[i] != '\0') {
        i++;
    }

    while (ct[j] != '\0') {
        s[i] = ct[j];
        i++;
        j++;
    }

    s[i] = '\0';
    return s;
}

/**
 * Concatenates a maximum of n characters from string ct to string s.
 *
 * @param s, the string to be operated.
 * @param ct, a constant string to be concatenated in s.
 * @param n, the quantity of characters of ct to be concatenated.
 * @return the string s.
 */
char *strncat(char *s, const char *ct, size_t n) {

    int i = 0;
    int j = 0;

    while (s[i] != '\0') {
        i++;
    }

    while (ct[j] != '\0' && j < n) {
        s[i] = ct[j];
        i++;
        j++;
    }

    s[i] = '\0';
    return s;
}

/**
 * Returns a pointer to the first appearance of c in cs.
 *
 * @param cs, the constant string to by analyzed.
 * @param c, the character c to be looked.
 * @return a pointer to the first position of c in cs or null.
 */
char *strchr(const char *cs, char c) {

    int i = 0;

    while (cs[i] != c && cs[i] != '\0') {
        i++;
    }

    return cs[i] == '\0' ? NULL : (cs + i);
}

/**
 * Returns a pointer to the last appearance of c in cs.
 *
 * @param cs, the constant string to be anlyzed.
 * @param c, the character c to be looked.
 * @return a pointer to the las positicion of c in cs or null.
 */
char *strrchr(const char *cs, char c) {

    int i = 0;
    int last = -1;

    while (cs[i] != '\0') {
        if (cs[i] == c) {
            last = i;
        }
        i++;
    }

    return last == -1 ? NULL : (cs + last);
}


/**
 * Compares strings cs and ct.
 *
 * @param cs, a constant string.
 * @param ct, a constant string to be compared with cs.
 * @return 0 if the strings are equivalent, -1 if cs < ct and 1 if cs > ct.
 */
int strcmp(const char *cs, const char *ct) {

    return strncmp(cs, ct, INT_MAX);
}

/**
 * Compares at most n characters of strings cs and ct.
 *
 * @param cs, a constant string.
 * @param ct, a constant string to be compared with cs.
 * @param n, the number of characters to be compared.
 * @return 0 if the strings are equivalent, -1 if cs < ct and 1 if cs > ct.
 */
int strncmp(const char *cs, const char *ct, size_t n) {

    int i = 0;

    while (cs[i] == ct[i] && cs[i] != '\0' && i < n) {
        i++;
    }

    if (i == n || cs[i] == ct[i]) {
        return 0;
    }

    return cs[i] < ct[i] ? -1 : 1;
}

/**
 *  Sets n characters of s with c.
 *  @param s, a void pointer to be written.
 *  @param c, the character chosen to fill s.
 *  @param n, the number of characters to be written.
 *  @return the void pointer s.
 */
void *memset(void *s, char c, size_t n) {

    int i = 0;

    while (i < n){
        *((char *) s + i) = c;
        i++;
    }

    return s;
}

/**
 * Copies n characters from ct to s.
 *
 * @param s, a void pointer. Destiny of the copy.
 * @param ct, a constant void pointer. Origin of the copy.
 * @param n, number of characters to be copied.
 * @return the void pointer s.
 */
void *memcpy(void *s, const void *ct, size_t n) {

    int i = 0;

    while (i < n) {
        *((char *) s + i) = *((char *)ct + i);
        i++;
    }

    return s;
}

/**
 * Returns pointer of first occurance of c in cs, checks n characters as most.
 *
 * @param cs, a constant void pointer to be analyzed.
 * @param c, the character to be looked.
 * @param n, the number of characters to check.
 * @return a void pointer pointing to the first appearance of c in cs or null.
 */
void *memchr(const void *cs, char c, size_t n){

    int i = 0;

    while (i < n && *((char *) cs + i) != c) {
        i++;
    }

    return *((char *) cs + i) == c ? (char *)cs + i : NULL;
}

/**
 * Compares the first n characters of cs and ct.
 *
 * @param cs, a constant void pointer.
 * @param ct, a constant void pointer which content will be compared to cs.
 * @param n, the number of characters to be compared.
 * @return 0 if cs is equivalent to ct, -1 if cs < ct or 1 if cs > ct.
 */
int memcmp(const void *cs, const void *ct, size_t n) {

    int i = 0;

    while (*((char *) cs + i) == *((char *) ct +i) && i < n) {
        i++;
    }

    if (*((char *) cs + i) == *((char *) ct + i)) {
        return 0;
    }

    return *((char *) cs + i) < *((char *) ct + i) ? -1 : 1;

}

/**
 * Reverses the string
 *
 * @param s, the string to be reversed.
 * @return the string s.
 */
char *reverse(char * s) {

    int i = 0;
    int len = strlen(s) - 1;
    char buff[len + 2];

    while (len >= 0) {
        buff[i] = s[len];
        len--;
        i++;
    }

    buff[i] = '\0';
    strcpy(s, buff);

    return s;

}

/*
 *  Checks if a string is effectively a number.
 *
 *  @param  str, the string to be analyzed.
 *  @return 1 if the string contains a number, 0 if not.
 */
int is_a_number(char* str) {
    
    int i = 0;
    
    while(str[0] == ' ') {
        str++;
    }

    if (str[0] == '-') {
        str++;
    }

    while(str[i] != '\0' && isdigit(str[i])) {
        i++;
    }

    return str[i] == '\0' && i != 0;
}

