


/**
 * Returns the lenght of a string withouth counting the final /0
 */
int strlen(const char *s) {

    int i = 0;

    while (*(s+i) != '\0')
        i++;
    return i;
}
