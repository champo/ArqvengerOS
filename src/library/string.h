#ifndef __string_header__

#define __string_header__

#include "type.h"

size_t strlen(const char *s);

char *strcpy(char *s, const char *ct);

char *strncpy(char *s, const char *ct, size_t n);

char *strcat(char *s, const char *ct);

char *strncat(char *s, const char *ct, size_t n);

char *strchr(const char *cs, char c);

char *strrchr(const char *cs, char c);

int strcmp(const char *cs, const char *ct);

int strncmp(const char *cs, const char *ct, size_t n);

void *memcpy(void *s, const void *ct, size_t n);

void *memchr(const void *cs, char c, size_t n);

void *memset(void *s, char c, size_t n);

int memcmp(const void *cs, const void *ct, size_t n);

char *reverse(char *s);

int is_a_number(char* str);

#endif
