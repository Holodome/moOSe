#pragma once

#include <types.h>
#include <varargs.h>

#define SEEK_SET 0
#define SEEK_END 1
#define SEEK_CUR 2

__attribute__((format(printf, 3, 4))) int snprintf(char *buffer, size_t size,
                                                   const char *fmt, ...);
int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args);
__attribute__((format(printf, 1, 2))) int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list args);

int kputc(int c);
int kputs(const char *str);

char *strerror(int errnum);
void perror(const char *msg);

int isdigit(int c);
int toupper(int c);

char *strpbrk(const char *string, const char *lookup);
size_t strspn(const char *string, const char *restrict lookup);
size_t strcspn(const char *string, const char *lookup);
char *strchr(const char *string, int symb);
char *strrchr(const char *string, int symb);

size_t strlcpy(char *dst, const char *src, size_t size);
