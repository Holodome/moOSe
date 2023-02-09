#pragma once

__attribute__((noreturn)) void __panic(void);

#define panic(...)                                                             \
    do {                                                                       \
        extern int kprintf(const char *, ...);                                 \
        kprintf("kernel panic:\n" __VA_ARGS__);                                \
        kprintf("at " __FILE__ ":" STRINGIFY(__LINE__) " in function %s\n",    \
                __PRETTY_FUNCTION__);                                          \
        __panic();                                                             \
    } while (0)
