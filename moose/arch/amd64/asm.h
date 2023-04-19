#pragma once

#include <types.h>

#define X86_FLAGS_IF 0x0200

static __forceinline void clac(void) {
    asm volatile("clac");
}

static __forceinline void nop(void) {
    asm volatile("nop");
}

static __forceinline void hlt(void) {
    asm volatile("hlt");
}

static __forceinline void cli(void) {
    asm volatile("cli");
}

static __forceinline void sti(void) {
    asm volatile("sti");
}

static __forceinline void pause(void) {
    asm volatile("pause");
}

static __forceinline u64 read_cr0(void) {
    u64 result;
    asm volatile("mov %%cr0, %%rax" : "=a"(result));
    return result;
}

static __forceinline u64 read_cr2(void) {
    u64 result;
    asm volatile("mov %%cr2, %%rax" : "=a"(result));
    return result;
}

static __forceinline u64 read_cr3(void) {
    u64 result;
    asm volatile("mov %%cr3, %%rax" : "=a"(result));
    return result;
}

static __forceinline void write_cr0(u64 value) {
    asm volatile("mov %0, %%cr0" : : "a"(value) : "memory");
}

static __forceinline void write_cr3(u64 value) {
    asm volatile("mov %0, %%cr3" : : "a"(value) : "memory");
}

static __forceinline u32 read_randr(void) {
    u32 result;
    asm volatile("1:\n"
                 "rdrand %0\n"
                 "jnc 1b\n"
                 : "=r"(result)::"cc");
    return result;
}

static __forceinline u32 read_rdseed(void) {
    u32 result;
    asm volatile("1:\n"
                 "rdseed %0\n"
                 "jnc 1b\n"
                 : "=r"(result)::"cc");
    return result;
}

static __forceinline u8 port_in8(u16 port) {
    u8 result;
    asm volatile("in %%dx, %%al" : "=a"(result) : "d"(port));
    return result;
}

static __forceinline u16 port_in16(u16 port) {
    u16 result;
    asm volatile("in %%dx, %%ax" : "=a"(result) : "d"(port));
    return result;
}

static __forceinline u32 port_in32(u16 port) {
    u32 result;
    asm volatile("inl %w1, %0" : "=a"(result) : "d"(port));
    return result;
}

static __forceinline void port_out8(u16 port, u8 data) {
    asm volatile("out %%al, %%dx" : : "a"(data), "d"(port));
}

static __forceinline void port_out16(u16 port, u16 data) {
    asm volatile("out %%ax, %%dx" : : "a"(data), "d"(port));
}

static __forceinline void port_out32(u16 port, u32 data) {
    asm volatile("outl %0, %w1" : : "a"(data), "d"(port));
}

static __forceinline void port_in8a(u16 port, u8 *array, u64 count) {
    asm volatile("rep; insb" : "+D"(array), "+c"(count) : "d"(port));
}

static __forceinline void port_in16a(u16 port, u16 *array, u64 count) {
    asm volatile("rep; insw" : "+D"(array), "+c"(count) : "d"(port));
}

static __forceinline void port_in32a(u16 port, u32 *array, u64 count) {
    asm volatile("rep; insd" : "+D"(array), "+c"(count) : "d"(port));
}

static __forceinline void port_out8a(u16 port, const u8 *array, u64 count) {
    asm volatile("rep; outsb"
                 : "+S"(array), "+c"(count)
                 : "d"(port)
                 : "memory");
}

static __forceinline void port_out16a(u16 port, const u16 *array, u64 count) {
    asm volatile("rep; outsw"
                 : "+S"(array), "+c"(count)
                 : "d"(port)
                 : "memory");
}

static __forceinline void port_out32a(u32 port, const u32 *array, u64 count) {
    asm volatile("rep; outsd"
                 : "+S"(array), "+c"(count)
                 : "d"(port)
                 : "memory");
}

static __forceinline u64 read_cpu_flags(void) {
    u64 flags;
    asm volatile("pushf\n"
                 "pop %0\n"
                 : "=rm"(flags)::"memory");
    return flags;
}
