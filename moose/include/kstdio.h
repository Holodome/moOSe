#pragma once

#include <types.h>
#include <varargs.h>

__attribute__((format(printf, 3, 4))) int snprintf(char *buffer, size_t size,
                                                   const char *fmt, ...);
int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args);
__attribute__((format(printf, 1, 2))) int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list args);

int kputc(int c);
int kputs(const char *str);

