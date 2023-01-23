#include "kmem.h"

void mem_copy(void *src_, void *dst_, u32 size) {
    u8 *src = src_;
    u8 *dst = dst_;
    while (size--)
        *dst++ = *src++;
}

void mem_set(void *dst_, u8 value, u32 size) {
    u8 *dst = dst_;
    while (size--)
        *dst++ = value;
}
