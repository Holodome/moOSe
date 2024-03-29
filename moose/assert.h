#pragma once

#include <moose/panic.h>
#include <moose/types.h>

// NOTE: Use %s instead of compile-time string join here because in case
// assertion contains % compilation fails
#define assert(_x)                                                             \
    do {                                                                       \
        if (!(_x))                                                             \
            panic("assertion '%s' failed\n", STRINGIFY(_x));                   \
    } while (0)

#define expects(_x)                                                            \
    do {                                                                       \
        if (!(_x))                                                             \
            panic("expects '%s' failed! this is a bug!\n", STRINGIFY(_x));     \
    } while (0)
