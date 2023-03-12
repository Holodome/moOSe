#pragma once

#define ETH_FRAME_MAX_SIZE 1522
#define ETH_FRAME_MIN_SIZE 64

#define ETH_PAYLOAD_MAX_SIZE 1500
#define ETH_PAYLOAD_MIN_SIZE 46

static inline u64 bswap64(u64 x) {
    return __builtin_bswap64(x);
}

static inline u32 bswap32(u32 x) {
    return __builtin_bswap32(x);
}

static inline u16 bswap16(u16 x) {
    return ((x & 0xff) << 8) | ((x & 0xff00) >> 8);
}
