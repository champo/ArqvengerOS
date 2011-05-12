#ifndef __string_header__

#define __string_header__

size_t strlen(const char *s);
char *strcpy(char *s, const char *ct);
char *strncpy(char *s, const char *ct, size_t n);
char *strcat(char *s, const char *ct);
char *strncat(char *s, const char *ct, size_t n);
char *strchr(const char *cs, char c);
char *strrchr(const char *cs, char c);

#endif
