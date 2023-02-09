#pragma once

#include <types.h>

__attribute__((noreturn)) static inline void hlt(void) {
    asm volatile("hlt");
    __builtin_unreachable();
}

static inline void cli(void) { asm volatile("cli" : : : "memory"); }
static inline void sti(void) { asm volatile("sti" : : : "memory"); }
static inline void pause(void) { asm volatile("pause"); }

static inline u64 read_cr0(void) {
    u64 result;
    asm("mov %%cr0, %%rax" : "=a"(result));
    return result;
}

static inline u64 read_cr2(void) {
    u64 result;
    asm("mov %%cr2, %%rax" : "=a"(result));
    return result;
}

static inline u64 read_cr3(void) {
    u64 result;
    asm("mov %%cr3, %%rax" : "=a"(result));
    return result;
}

static inline void write_cr0(u64 value) {
    asm volatile("mov %0, %%cr0" : : "a"(value) : "memory");
}

static inline void write_cr3(u64 value) {
    asm volatile("mov %0, %%cr3" : : "a"(value) : "memory");
}

static inline u32 read_randr(void) {
    u32 result;
    asm("1:\n"
        "rdrand %0\n"
        "jnc 1b\n"
        : "=r"(result)::"cc");
    return result;
}

static inline u32 read_rdseed(void) {
    u32 result;
    asm("1:\n"
        "rdseed %0\n"
        "jnc 1b\n"
        : "=r"(result)::"cc");
    return result;
}

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
