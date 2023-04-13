#pragma once

#include <types.h>

struct cpu_registers;

__noreturn void __panic(void);
__noreturn void ___panic(void);

#define panic(...)                                                             \
    do {                                                                       \
        extern int kprintf(const char *, ...);                                 \
        kprintf("kernel panic:\n" __VA_ARGS__);                                \
        kprintf("at " __FILE__ ":" STRINGIFY(__LINE__) " in function %s\n",    \
                __PRETTY_FUNCTION__);                                          \
        __panic();                                                             \
    } while (0)

#define _panic(...)                                                            \
    do {                                                                       \
        extern int kprintf(const char *, ...);                                 \
        kprintf("kernel panic:\n" __VA_ARGS__);                                \
        kprintf("at " __FILE__ ":" STRINGIFY(__LINE__) " in function %s\n",    \
                __PRETTY_FUNCTION__);                                          \
        ___panic();                                                            \
    } while (0)
