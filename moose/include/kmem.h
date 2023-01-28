#pragma once

#include "types.h"

#define memcpy __builtin_memcpy
#define memset __builtin_memset
#define memmove __builtin_memmove

u32 strlen(const char *str);
