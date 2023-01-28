#pragma once

#include "types.h"

#define memcpy __builtin_memcpy
#define memset __builtin_memset
#define memmove __builtin_memmove

static inline u32 strlen(const char *str) {
    const char *cur = str;
    while (*++cur);
    return cur - str;
}
