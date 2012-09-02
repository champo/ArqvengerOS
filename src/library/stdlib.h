#ifndef __stdlib_header__

#define __stdlib_header__

#include "library/stddef.h"

int atoi(const char *s);

int itoa(char *s, int n);

unsigned int atou(const char *s);

int utoa(char *s, unsigned int n);

int rand(void);

void srand(unsigned int seed);

int parseoct(char* string);

size_t processStackSize(void);

#define RAND_MAX 268435456

#endif

