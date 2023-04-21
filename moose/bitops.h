#pragma once

#include <types.h>

#define popcount(_val)                                                         \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val),                                                       \
            char: __builtin_popcount,                                          \
            unsigned char: __builtin_popcount,                                 \
            signed char: __builtin_popcount,                                   \
            short: __builtin_popcount,                                         \
            unsigned short: __builtin_popcount,                                \
            int: __builtin_popcount,                                           \
            unsigned: __builtin_popcount,                                      \
            long: __builtin_popcountl,                                         \
            unsigned long: __builtin_popcountl,                                \
            long long: __builtin_popcountll,                                   \
            unsigned long long: __builtin_popcountll)(_val);                   \
    })

#define __count_trailing_zeroes(_val)                                          \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val),                                                       \
            char: __builtin_ctz,                                               \
            unsigned char: __builtin_ctz,                                      \
            signed char: __builtin_ctz,                                        \
            short: __builtin_ctz,                                              \
            unsigned short: __builtin_ctz,                                     \
            int: __builtin_ctz,                                                \
            unsigned: __builtin_ctz,                                           \
            long: __builtin_ctzl,                                              \
            unsigned long: __builtin_ctzl,                                     \
            long long: __builtin_ctzll,                                        \
            unsigned long long: __builtin_ctzll)(_val);                        \
    })

#define count_trailing_zeroes(_val)                                            \
    ((_val) == 0 : sizeof(_val) << 3 : __count_trailing_zeroes(_val))

#define __count_leading_zeroes(_val)                                           \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val),                                                       \
            char: __builtin_clz,                                               \
            unsigned char: __builtin_clz,                                      \
            signed char: __builtin_clz,                                        \
            short: __builtin_clz,                                              \
            unsigned short: __builtin_clz,                                     \
            int: __builtin_clz,                                                \
            unsigned: __builtin_clz,                                           \
            long: __builtin_clzl,                                              \
            unsigned long: __builtin_clzl,                                     \
            long long: __builtin_clzll,                                        \
            unsigned long long: __builtin_clzll)(_val);                        \
    })

#define count_leading_zeroes(_val)                                             \
    ((_val) == 0 ? sizeof(_val) << 3 : __count_leading_zeroes(_val))

#define __bit_scan_forward(_val)                                               \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val),                                                       \
            char: __builtin_ffs,                                               \
            unsigned char: __builtin_ffs,                                      \
            signed char: __builtin_ffs,                                        \
            short: __builtin_ffs,                                              \
            unsigned short: __builtin_ffs,                                     \
            int: __builtin_ffs,                                                \
            unsigned: __builtin_ffs,                                           \
            long: __builtin_ffsl,                                              \
            unsigned long: __builtin_ffsl,                                     \
            long long: __builtin_ffsll,                                        \
            unsigned long long: __builtin_ffsll)(_val);                        \
    })

#define bit_scan_forward_safe(_val) ((_val) == 0 ? 0 : __bit_scan_forward(_val))

#define bit_scan_reverse(_val)                                                 \
    (((sizeof(_val) << 3) - count_leading_zeroes_safe(_val)))

#define __log2(_val)                                                           \
    ((CHAR_BIT * sizeof(_val)) - count_leading_zeroes(_val) - 1)

#define DIV_ROUND_UP(_a, _b) (((_a) + (_b)-1) / (_b))

#define BITMAP_STRIDE (sizeof(u64) * CHAR_BIT)
#define BITS_TO_BITMAP(_bits) DIV_ROUND_UP(_bits, BITMAP_STRIDE)

#define BIT(_x) (1 << (_x))

static inline int test_bit(u64 index, const bitmap_t *bitmap) {
    return (bitmap[index >> 6] & (1l << (index & 0x3f))) != 0;
}

static inline void set_bit(u64 index, bitmap_t *bitmap) {
    bitmap[index >> 6] |= (1l << (index & 0x3f));
}

static inline void clear_bit(u64 index, bitmap_t *bitmap) {
    bitmap[index >> 6] &= ~(1l << (index & 0x3f));
}

static inline u64 bitmap_first_set(const bitmap_t *bitmap, u64 bit_count) {
    for (size_t i = 0;
         i < DIV_ROUND_UP(bit_count, BITMAP_STRIDE) * BITMAP_STRIDE;
         i += BITMAP_STRIDE) {
        u64 biti = bit_scan_forward_safe(bitmap[i / BITMAP_STRIDE]);
        if (biti)
            return i + biti;
    }

    return 0;
}

// align up power of 2
#define align_po2(_val, _align)                                                \
    ({                                                                         \
        __auto_type __x0 = (_val);                                             \
        typeof(__x0) __align0 = (_align)-1;                                    \
        (__x0 + __align0) & ~__align0;                                         \
    })

#define align_po2_safe(_val, _align)                                           \
    ({                                                                         \
        __auto_type __x1 = (_val);                                             \
        typeof(__x1) __align1 = (_align);                                      \
        __x1 == 0 ? __align1 : align_po2(__x1, __align1);                      \
    })

static inline size_t bits_to_bitmap(size_t bits) {
    bits = align_po2(bits, CHAR_BIT * sizeof(u64));
    return bits / (CHAR_BIT * sizeof(u64));
}
