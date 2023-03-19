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

#define __forceinline inline __attribute__((always_inline))
#define __nodiscard __attribute__((warn_unused_result))
#define __unlikely(_x) __builtin_expect(_x, 0)
#define __unused __attribute__((unused))
#define __used __attribute__((used))
#define __aligned(_x) __attribute__((aligned(_x)))
#define __noreturn __attribute__((noreturn))
#define __packed __attribute__((packed))
#define __printf(_a, _b) __attribute__((format(printf, _a, _b)))
