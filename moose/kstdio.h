#pragma once

#include <moose/types.h>
#include <moose/varargs.h>

__printf(3, 4) int snprintf(char *buffer, size_t size, const char *fmt, ...);
int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args);
__printf(1, 2) int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list args);

int init_kstdio(void);
