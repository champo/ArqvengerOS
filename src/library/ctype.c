
/**
 * checks if the character represents a digit
 */
int isdigit(int c){
    
    return((c >= '0' && c <= '9') ? 1 : 0);
}

/**
 * checks if the character represents a space
 */
int isspace(int c){

    return(c == ' ' ? 1 : 0);
}
