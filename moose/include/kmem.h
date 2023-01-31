#pragma once

#include <types.h>

static inline void *memcpy(void *dst_, const void *src_, size_t c) {
    u8 *dst = dst_;
    const u8 *src = src_;
    while (c--)
        *dst++ = *src++;

    return dst_;
}

static inline void *memset(void *dst_, int ch, size_t c) {
    u8 *dst = dst_;
    while (c--)
        *dst++ = ch;

    return dst_;
}

static inline void *memmove(void *dst_, const void *src_, size_t c) {
    u8 *dst = dst_;
    const u8 *src = src_;
    if (dst == src) {
        return dst;
    }

    if (dst < src) {
        for (; c; --c)
            *dst++ = *src++;
    } else {
        while (c) {
            --c;
            dst[c] = src[c];
        }
    }

    return dst_;
}

static inline int memcmp(const void *l_, const void *r_, size_t c) {
    const u8 *l = l_;
    const u8 *r = r_;
    while (c--) {
        int d = *l - *r;
        if (d)
            return d;
    }

    return 0;
}

static inline size_t strlen(const char *str) {
    const char *cur = str;
    while (*++cur)
        ;
    return cur - str;
}
