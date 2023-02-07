#pragma once

#include <panic.h>
#include <types.h>

#define assert(_x)                                                             \
    do {                                                                       \
        if (!(_x)) {                                                           \
            panic("assertion '" STRINGIFY(_x) "' failed\n");                   \
        }                                                                      \
    } while (0)
