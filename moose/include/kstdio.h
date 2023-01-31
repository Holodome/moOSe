#pragma once

#include <types.h>
#include <varargs.h>

int snprintf(char *buffer, size_t size, const char *fmt, ...);
int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args);
int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list args);

int kputc(int c);
int kputs(const char *str);
