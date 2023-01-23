#include "ports.h"

u8 port_u8_in(u16 port) {
    u8 result;
    asm volatile("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

u16 port_u16_in(u16 port) {
    u16 result;
    asm volatile("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

void port_u8_out(u16 port, u8 data) { asm volatile("out %%al, %%dx" : : "a"(data), "d"(port)); }
void port_u16_out(u16 port, u16 data) { asm volatile("out %%ax, %%dx" : : "a"(data), "d"(port)); }
