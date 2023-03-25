#pragma once

#include <types.h>

static inline u16 read_be16(const u8 *src) {
    return ((u16)src[0] << 8) | (u16)src[1];
}

static inline u16 read_le16(const u8 *src) {
    return ((u16)src[1] << 8) | (u16)src[0];
}

static inline u32 read_be32(const u8 *src) {
    return ((u32)src[0] << 24) | ((u32)src[1] << 16) | ((u32)src[2] << 8) |
           (u32)src[3];
}

static inline u32 read_le32(const u8 *src) {
    return ((u32)src[3] << 24) | ((u32)src[2] << 16) | ((u32)src[1] << 8) |
           (u32)src[0];
}

static inline u64 read_be64(const u8 *src) {
    return ((u64)src[0] << 56) | ((u64)src[1] << 48) | ((u64)src[2] << 40) |
           ((u64)src[3] << 32) | ((u64)src[4] << 24) | ((u64)src[5] << 16) |
           ((u64)src[6] << 8) | (u64)src[7];
}

static inline u64 read_le64(const u8 *src) {
    return ((u64)src[7] << 56) | ((u64)src[6] << 48) | ((u64)src[5] << 40) |
           ((u64)src[4] << 32) | ((u64)src[3] << 24) | ((u64)src[2] << 16) |
           ((u64)src[1] << 8) | (u64)src[0];
}

static inline void write_be16(u8 *dst, u16 data) {
    *dst++ = data >> 8;
    *dst = data & 0xff;
}

static inline void write_le16(u8 *dst, u16 data) {
    *dst++ = data & 0xff;
    *dst = data >> 8;
}

static inline void write_be32(u8 *dst, u32 data) {
    *dst++ = data >> 24;
    *dst++ = (data >> 16) & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst = data & 0xff;
}

static inline void write_le32(u8 *dst, u32 data) {
    *dst++ = data & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst = data >> 24;
}

static inline void write_be64(u8 *dst, u64 data) {
    *dst++ = data >> 56;
    *dst++ = (data >> 48) & 0xff;
    *dst++ = (data >> 40) & 0xff;
    *dst++ = (data >> 32) & 0xff;
    *dst++ = (data >> 24) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst = data & 0xff;
}

static inline void write_le64(u8 *dst, u64 data) {
    *dst++ = data & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst++ = (data >> 24) & 0xff;
    *dst++ = (data >> 32) & 0xff;
    *dst++ = (data >> 40) & 0xff;
    *dst++ = (data >> 48) & 0xff;
    *dst = data >> 56;
}

static inline u16 htobe16(u16 a) {
    u16 result;
    write_be16((u8 *)&result, a);
    return result;
}

static inline u16 htole16(u16 a) {
    u16 result;
    write_le16((u8 *)&result, a);
    return result;
}

static inline u16 be16toh(u16 a) {
    return read_be16((u8 *)&a);
}

static inline u16 le16toh(u16 a) {
    return read_le16((u8 *)&a);
}

static inline u32 htobe32(u32 a) {
    u32 result;
    write_be32((u8 *)&result, a);
    return result;
}

static inline u32 htole32(u32 a) {
    u32 result;
    write_le32((u8 *)&result, a);
    return result;
}

static inline u32 be32toh(u32 a) {
    return read_be32((u8 *)&a);
}

static inline u32 le32toh(u32 a) {
    return read_le32((u8 *)&a);
}

static inline u64 htobe64(u64 a) {
    u64 result;
    write_be64((u8 *)&result, a);
    return result;
}

static inline u64 htole64(u64 a) {
    u64 result;
    write_le64((u8 *)&result, a);
    return result;
}

static inline u64 be64toh(u64 a) {
    return read_be64((u8 *)&a);
}

static inline u64 le64toh(u64 a) {
    return read_le64((u8 *)&a);
}
