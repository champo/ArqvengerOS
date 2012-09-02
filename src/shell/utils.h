#ifndef __SHELL_UTILS__
#define __SHELL_UTILS__


#define cleanbuffer() while(getchar()!='\n')

void askForPasswd(const char* prompt, char* passwd);

void askForInput(const char* prompt, char* buffer);

#endif
