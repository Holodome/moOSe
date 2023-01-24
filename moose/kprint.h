#pragma once

#include "types.h"

__attribute__((format(printf, 3, 4))) int snprintf(char *buf, size_t buf_size,
                                                   const char *fmt, ...);
int vsnprintf(char *buf, size_t buf_size, const char *fmt, va_list lst);
__attribute__((format(printf, 1, 2))) int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list lst);
