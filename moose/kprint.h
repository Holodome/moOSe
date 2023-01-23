#pragma once

#include "types.h"
#include "varargs.h"

__attribute__((format(printf, 3, 4))) u32 snprintf(char *buf, u32 buf_size,
                                                   const char *fmt, ...);
u32 vsnprintf(char *buf, u32 buf_size, va_list lst);
__attribute__((format(printf, 1, 2))) void kprintf(const char *fmt, ...);
void kvprintf(const char *fmt, va_list lst);
