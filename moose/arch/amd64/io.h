#pragma once

#include "types.h"

static inline u8 port_in8(u16 port) {
    u8 result;
    asm volatile("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

static inline u16 port_in16(u16 port) {
    u16 result;
    asm volatile("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

static inline void port_out8(u16 port, u8 data) {
    asm volatile("out %%al, %%dx" : : "a"(data), "d"(port));
}

static inline void port_out16(u16 port, u16 data) {
    asm volatile("out %%ax, %%dx" : : "a"(data), "d"(port));
}
