#ifndef __SHELL_UTILS__
#define __SHELL_UTILS__


#define cleanbuffer() while(getchar()!='\n')

void askForPasswd(char* prompt, char* passwd);

void askForInput(char* prompt, char* buffer);

#endif
