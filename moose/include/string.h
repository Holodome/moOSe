#pragma once

#include <types.h>

void *memcpy(void *dst, const void *src, size_t c);
void *memset(void *dst, int ch, size_t c);
void *memmove(void *dst, const void *src, size_t c);
int memcmp(const void *l, const void *r, size_t c);

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
int strcmp(const char *, const char *);

size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
