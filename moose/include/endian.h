#pragma once

#include <types.h>

static inline u16 read_le16(const u8 *src) {
    return ((u16)src[1] << 8) | (u16)src[0];
}

static inline u32 read_le32(const u8 *src) {
    return ((u32)src[3] << 24) | ((u32)src[2] << 16) | ((u32)src[1] << 8) |
           (u32)src[0];
}

static inline u64 read_le64(const u8 *src) {
    return ((u64)src[7] << 56) | ((u64)src[6] << 48) | ((u64)src[5] << 40) |
           ((u64)src[4] << 32) | ((u64)src[3] << 24) | ((u64)src[2] << 16) |
           ((u64)src[1] << 8) | (u64)src[0];
}

static inline void write_le16(u8 *dst, u16 data) {
    *dst++ = data & 0xff;
    *dst = data >> 8;
}

static inline void write_le32(u8 *dst, u32 data) {
    *dst++ = data & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst = data >> 24;
}

static inline void write_le64(u8 *dst, u64 data) {
    *dst++ = data & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst++ = (data >> 24) & 0xff;
    *dst++ = (data >> 32) & 0xff;
    *dst++ = (data >> 40) & 0xff;
    *dst++ = (data >> 48) & 0xff;
    *dst++ = data >> 56;
}

static inline u16 __le16(u16 src) {
    u16 result;
    write_le16((u8 *)&result, src);
    return result;
}

static inline u32 __le32(u32 src) {
    u32 result;
    write_le32((u8 *)&result, src);
    return result;
}

static inline u64 __le64(u64 src) {
    u64 result;
    write_le64((u8 *)&result, src);
    return result;
}
