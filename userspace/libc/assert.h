#pragma once

#include <sys/cdefs.h>

#define __stringify_(_x) #_x
#define __stringify(_x) __stringify_(_x)

#ifdef NDEBUG
#define assert(_cond) ((void)0)
#else
#define assert(_cond)                                                          \
    (__builtin_expect(!(_cond), 0)                                             \
         ? __assertion_failed(#_cond, "\n" __FILE__ ":" __stringify(__LINE__))  \
         : (void)0)
#endif

#undef __stringify_
#undef __stringify

__BEGIN_DECLS

__attribute__((noreturn)) void __assertion_failed(const char *msg);
