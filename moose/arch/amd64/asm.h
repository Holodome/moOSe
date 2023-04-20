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

static __forceinline u64 read_msr(u32 msr) {
    u32 lo, hi;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));
    return ((u64)hi << 32) | lo;
}

static __forceinline void write_msr(u32 msr, u64 value) {
    u32 lo = value & 0xffffffff;
    u32 hi = value >> 32;
    asm volatile("wrmsr" ::"a"(lo), "d"(hi), "c"(msr));
}

struct cpuid {
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
};

static __forceinline void cpuid(u32 func, u32 ecx, struct cpuid *id) {
    asm volatile("cpuid"
                 : "=a"(id->eax), "=b"(id->ebx), "=c"(id->ecx), "=d"(id->edx)
                 : "a"(func), "c"(ecx));
}

// clang-format off
#define __read_gs(_n, _l)                                                      \
    static __forceinline __nodiscard u##_n read_gs##_n(u32 offset) {           \
        u##_n result;                                                          \
        asm volatile("mov" _l " %%gs:%a[off], %[val]"                          \
                     : [val] "=r"(result)                                      \
                     : [off] "ir"(offset));                                    \
        return result;                                                         \
    }

__read_gs(8, "b") 
__read_gs(16, "w") 
__read_gs(32, "l") 
__read_gs(64, "q")

#undef __read_gs

static __forceinline __nodiscard void *read_gs_ptr(u32 offset) {
    return (void *)(uintptr_t)read_gs64(offset);
}

#define read_gs_int read_gs32

#define __write_gs(_n, _l)                                                     \
    static __forceinline void write_gs##_n(u32 offset, u##_n value) {          \
        asm volatile("mov" _l " %[val], %%gs:%a[off]" ::[off] "ir"(offset),    \
                     [val] "r"(value));                                        \
    }

__write_gs(8, "b")
__write_gs(16, "w")
__write_gs(32, "l")
__write_gs(64, "q")

#undef __write_gs

static __forceinline void write_gs_ptr(u32 offset, const void *ptr){ 
    write_gs64(offset, (u64)(uintptr_t)ptr);
}

#define write_gs_int write_gs32

// clang-format on
