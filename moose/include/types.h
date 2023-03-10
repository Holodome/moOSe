#pragma once

#include <arch/amd64/types.h>

#define static_assert(_x) _Static_assert(_x, #_x)
#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*_arr))
#define offsetof(_struct, _field) ((size_t)(&((_struct *)(0))->_field))
#define container_of(_ptr, _type, _member)                                     \
    ({                                                                         \
        const typeof(((_type *)0)->_member) *__mptr = (_ptr);                  \
        (_type *)((char *)__mptr - offsetof(_type, _member));                  \
    })

#define STRINGIFY_(_x) #_x
#define STRINGIFY(_x) STRINGIFY_(_x)
