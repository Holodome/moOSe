#pragma once

__attribute__((noreturn)) void __panic(const char *file, unsigned line,
                                       const char *function);

#define panic(...)                                                             \
    do {                                                                       \
        kprintf("kernel panic: " __VA_ARGS__);                                 \
        __panic(__FILE__, __LINE__, __PRETTY_FUNCTION__);                      \
    } while (0)
