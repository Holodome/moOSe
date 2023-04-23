#pragma once

#include <arch/amd64/types.h>

#define static_assert(_x) _Static_assert(_x, #_x)
#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*(_arr)))
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
#define __unlikely(_x) __builtin_expect(!!(_x), 0)
#define __likely(_x) __builtin_expect(!(_x), 0)
#define __unused __attribute__((unused))
#define __used __attribute__((used))
#define __aligned(_x) __attribute__((aligned(_x)))
#define __noreturn __attribute__((noreturn))
#define __packed __attribute__((packed))
#define __printf(_a, _b) __attribute__((format(printf, _a, _b)))
#define __noinline __attribute__((noinline))
#define __naked __attribute__((naked))

static_assert(sizeof(i8) == 1);
static_assert(sizeof(u8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(i64) == 8);
static_assert(sizeof(u64) == 8);

typedef u32 uid_t;
typedef u32 gid_t;
typedef u64 ino_t;
typedef i64 off_t;

typedef u32 blkcnt_t;
typedef u32 blksize_t;
typedef u32 dev_t;
typedef u16 mode_t;
typedef u32 nlink_t;

typedef i64 time_t;
typedef u32 useconds_t;
typedef u32 suseconds_t;
typedef i32 iseconds_t;
typedef u32 clock_t;

typedef i64 off_t;
typedef u32 pid_t;
typedef u64 bitmap_t;
