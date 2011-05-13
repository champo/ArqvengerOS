
/**
 * checks if the character represents a digit
 */
int isdigit(int c) {
    
    return(c >= '0' && c <= '9');
}

/**
 * checks if the character represents a space
 */
int isspace(int c) {

    return(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}
