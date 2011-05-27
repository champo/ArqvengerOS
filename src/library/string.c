#include "type.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "limits.h"
#include "string.h"
#include "library/stdlib.h"


/**
 * Returns the lenght of a string withouth counting the final /0
 */
size_t strlen(const char *s) {

    int i = 0;

    while (*(s+i) != '\0') {
        i++;
    }
    return i;
}

/**
 * Copies the string ct to string s
 */
char *strcpy(char *s, const char *ct) {
    int i = 0;

    while(ct[i] != '\0'){
        s[i] = ct[i];
        i++;
    }

    s[i] = '\0';
    return s;
}

/**
 * Copies a maximum of n characters from ct to n
 */
char *strncpy(char *s, const char *ct, size_t n) {

    int i;

    for(i = 0; i < n && ct[i] != '\0'; i++) {
        s[i] = ct[i];
    }
    s[i] = '\0';

    return s;
}

/**
 * Concatenates string s with string ct
 */
char *strcat(char *s, const char *ct) {

    int i = 0;
    int j = 0;

    while(s[i] != '\0') {
        i++;
    }

    while(ct[j] != '\0') {
        s[i] = ct[j];
        i++;
        j++;
    }

    s[i] = '\0';
    return s;
}

/**
 * Concatenates a maximum of n characters from string ct to string s
 */
char *strncat(char *s, const char *ct, size_t n) {

    int i = 0;
    int j = 0;

    while(s[i] != '\0') {
        i++;
    }

    while(ct[j] != '\0' && j < n) {
        s[i] = ct[j];
        i++;
        j++;
    }

    s[i] = '\0';
    return s;
}

/**
 * Returns a pointer to the first appearance of c in cs
 */
char *strchr(const char *cs, char c) {

    int i = 0;

    while(cs[i] != c && cs[i] != '\0') {
        i++;
    }

    return(cs[i] == '\0' ? NULL : (cs + i));
}

/**
 * Returns a pointer to the last appearance of c in cs
 */
char *strrchr(const char *cs, char c) {

    int i = 0;
    int last = -1;

    while(cs[i] != '\0') {
        if(cs[i] == c) {
            last = i;
        }
        i++;
    }

    return(last == -1 ? NULL : (cs + last));
}


/**
 * Compares strings cs and ct.
 */
int strcmp(const char *cs, const char *ct) {

    return strncmp(cs, ct, INT_MAX);
}

/**
 * Compares at most n characters of strings cs and ct.
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
 *  Sets n characters of s with c
 */
void *memset(void *s, char c, size_t n) {

    int i = 0;

    while(i < n){
        *((char *) s + i) = c;
        i++;
    }

    return s;
}

/**
 * Copies n characters from ct to s
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
 * Returns pointer of first occurance of c in cs, checks n characters as most
 */
void *memchr(const void *cs, char c, size_t n){

    int i = 0;

    while(i < n && *((char *) cs + i) != c) {
        i++;
    }

    return *((char *) cs + i) == c ? (char *)cs + i : NULL;
}

/**
 * Compares the first n characters of cs and ct
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
