#include "type.h"
#include "library/stdio.h"


/**
 * Returns the lenght of a string withouth counting the final /0
 */
size_t strlen(const char *s) {

    int i = 0;

    while (*(s+i) != '\0')
        i++;
    return i;
}

/**
 * Copies the string ct to string s
 */
char *strcpy(char *s, const char *ct){
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
char *strncpy(char *s, const char *ct, size_t n){
    
    int i;

    for(i = 0; i<n || ct[i] != '\0'; i++)
        s[i] = ct[i];
    s[i] = '\0';
    
    return s;
}

/** 
 * Concatenates string s with string ct
 */
char *strcat(char *s, const char *ct){
    
    int i = 0;
    int j = 0;

    while(s[i] != '\0')
        i++;

    while(ct[j] != '\0'){
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
char *strncat(char *s, const char *ct, size_t n){
    
    int i = 0;
    int j = 0;

    while(s[i] != '\0')
        i++;

    while(ct[j] != '\0' || j<n){
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
char *strchr(const char *cs, char c){

    int i = 0;

    while(cs[i] != c && cs[i] != '\0')
        i++;

    return(cs[i] == '\0' ? NULL : (cs + i));
}

/** 
 * Returns a pointer to the last appearance of c in cs
 */
char *strrchr(const char *cs, char c){

    int i = 0;
    int last = -1;

    while(cs[i] != '\0'){
        if(cs[i] == c)
            last = i;        
        i++;
    }

    return(last == -1 ? NULL : (cs + i));
}
