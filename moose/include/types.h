#pragma once

#include <arch/amd64/types.h>

typedef ssize_t off_t;
#define static_assert(_x) _Static_assert(_x, #_x)
#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*_arr))
