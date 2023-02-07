#pragma once

#include <panic.h>
#include <types.h>

// NOTE: Use %s instead of compile-time string join here because in case
// assertion contains % compilation fails
#define assert(_x)                                                             \
    do {                                                                       \
        if (!(_x)) {                                                           \
            panic("assertion '%s' failed\n", STRINGIFY(_x));                   \
        }                                                                      \
    } while (0)
