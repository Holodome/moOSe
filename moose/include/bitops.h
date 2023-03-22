#pragma once

#include <types.h>

#define popcount(_val)                                                         \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val), char                                                  \
                 : __builtin_popcount, unsigned char                           \
                 : __builtin_popcount, signed char                             \
                 : __builtin_popcount, short                                   \
                 : __builtin_popcount, unsigned short                          \
                 : __builtin_popcount, int                                     \
                 : __builtin_popcount, unsigned                                \
                 : __builtin_popcount, long                                    \
                 : __builtin_popcountl, unsigned long                          \
                 : __builtin_popcountl, long long                              \
                 : __builtin_popcountll, unsigned long long                    \
                 : __builtin_popcountll)(_val);                                \
    })

#define count_trailing_zeroes(_val)                                            \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val), char                                                  \
                 : __builtin_ctz, unsigned char                                \
                 : __builtin_ctz, signed char                                  \
                 : __builtin_ctz, short                                        \
                 : __builtin_ctz, unsigned short                               \
                 : __builtin_ctz, int                                          \
                 : __builtin_ctz, unsigned                                     \
                 : __builtin_ctz, long                                         \
                 : __builtin_ctzl, unsigned long                               \
                 : __builtin_ctzl, long long                                   \
                 : __builtin_ctzll, unsigned long long                         \
                 : __builtin_ctzll)(_val);                                     \
    })

#define count_trailing_zeroes_safe(_val)                                       \
    ((_val) == 0 : sizeof(_val) << 3 : count_trailing_zeroes(_val))

#define count_leading_zeroes(_val)                                             \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val), char                                                  \
                 : __builtin_clz, unsigned char                                \
                 : __builtin_clz, signed char                                  \
                 : __builtin_clz, short                                        \
                 : __builtin_clz, unsigned short                               \
                 : __builtin_clz, int                                          \
                 : __builtin_clz, unsigned                                     \
                 : __builtin_clz, long                                         \
                 : __builtin_clzl, unsigned long                               \
                 : __builtin_clzl, long long                                   \
                 : __builtin_clzll, unsigned long long                         \
                 : __builtin_clzll)(_val);                                     \
    })

#define count_leading_zeroes_safe(_val)                                        \
    ((_val) == 0 ? sizeof(_val) << 3 : count_leading_zeroes(_val))

#define bit_scan_forward(_val)                                                 \
    ({                                                                         \
        static_assert(sizeof(_val) <= sizeof(unsigned long long));             \
        _Generic((_val), char                                                  \
                 : __builtin_ffs, unsigned char                                \
                 : __builtin_ffs, signed char                                  \
                 : __builtin_ffs, short                                        \
                 : __builtin_ffs, unsigned short                               \
                 : __builtin_ffs, int                                          \
                 : __builtin_ffs, unsigned                                     \
                 : __builtin_ffs, long                                         \
                 : __builtin_ffsl, unsigned long                               \
                 : __builtin_ffsl, long long                                   \
                 : __builtin_ffsll, unsigned long long                         \
                 : __builtin_ffsll)(_val);                                     \
    })

#define bit_scan_reverse(_val)                                                 \
    (((sizeof(_val) << 3) - count_leading_zeroes_safe(_val)))

#define __log2(_val)                                                           \
    ((CHAR_BIT * sizeof(_val)) - count_leading_zeroes(_val) - 1)

#define DIV_ROUND_UP(_a, _b) (((_a) + (_b)-1) / (_b))
#define BITS_TO_BITMAP(_bits) DIV_ROUND_UP(_bits, sizeof(long) * CHAR_BIT)

#define BIT(_x) (1 << (_x))

static inline int test_bit(u64 index, const u64 *bitmap) {
    return (bitmap[index >> 6] & (1l << (index & 0x3f))) != 0;
}

static inline void set_bit(u64 index, u64 *bitmap) {
    bitmap[index >> 6] |= (1l << (index & 0x3f));
}

static inline void clear_bit(u64 index, u64 *bitmap) {
    bitmap[index >> 6] &= ~(1l << (index & 0x3f));
}

// align up power of 2
static inline size_t align_po2(size_t val, size_t align) {
    val += align - 1;
    val &= ~(align - 1);
    return val;
}

static inline size_t bits_to_bitmap(size_t bits) {
    bits = align_po2(bits, CHAR_BIT * sizeof(u64));
    return bits / (CHAR_BIT * sizeof(u64));
}
