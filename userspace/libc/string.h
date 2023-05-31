#pragma once 

#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t c);
void *memset(void *dst, int ch, size_t c);
void *memmove(void *dst, const void *src, size_t c);
int memcmp(const void *l, const void *r, size_t c);
void *memchr(const void *src, int ch, size_t c);

char *strpbrk(const char *string, const char *lookup);
size_t strspn(const char *string, const char *restrict lookup);
size_t strcspn(const char *string, const char *lookup);
char *strchr(const char *string, int symb);
char *strrchr(const char *string, int symb);
size_t strlen(const char *str);
char *strncpy(char *dst, const char *src, size_t c);
char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t c);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t c);
char *strstr(const char *s, const char *a);

size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
char *strchrnul(const char *s, int c);
char *strnchr(const char *s, int ch, size_t c);
char *strnchrnul(const char *s, int ch, size_t c);
size_t strnlen(const char *s, size_t c);
char *strsep(char **sp, const char *sep);
