#pragma once

#include <types.h>

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

static inline u8 cmos_read(u8 idx) {
    port_out8(0x70, idx);
    return port_in8(0x71);
}

static inline void cmos_write(u8 idx, u8 data) {
    port_out8(0x70, idx);
    port_out8(0x71, data);
}
