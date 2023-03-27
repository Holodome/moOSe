#pragma once

#include <types.h>

static __forceinline u16 read_be16(const u8 *src) {
    return ((u16)src[0] << 8) | (u16)src[1];
}

static __forceinline u32 read_be32(const u8 *src) {
    return ((u32)src[0] << 24) | ((u32)src[1] << 16) | ((u32)src[2] << 8) |
           (u32)src[3];
}

static __forceinline u64 read_be64(const u8 *src) {
    return ((u64)src[0] << 56) | ((u64)src[1] << 48) | ((u64)src[2] << 40) |
           ((u64)src[3] << 32) | ((u64)src[4] << 24) | ((u64)src[5] << 16) |
           ((u64)src[6] << 8) | (u64)src[7];
}

static __forceinline void write_be16(u8 *dst, u16 data) {
    *dst++ = data >> 8;
    *dst = data & 0xff;
}

static __forceinline void write_be32(u8 *dst, u32 data) {
    *dst++ = data >> 24;
    *dst++ = (data >> 16) & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst = data & 0xff;
}

static __forceinline void write_be64(u8 *dst, u64 data) {
    *dst++ = data >> 56;
    *dst++ = (data >> 48) & 0xff;
    *dst++ = (data >> 40) & 0xff;
    *dst++ = (data >> 32) & 0xff;
    *dst++ = (data >> 24) & 0xff;
    *dst++ = (data >> 16) & 0xff;
    *dst++ = (data >> 8) & 0xff;
    *dst = data & 0xff;
}

static __forceinline u16 htobe16(u16 a) {
    u16 result;
    write_be16((u8 *)&result, a);
    return result;
}

static __forceinline u16 be16toh(u16 a) { return read_be16((u8 *)&a); }
static __forceinline u32 htobe32(u32 a) {
    u32 result;
    write_be32((u8 *)&result, a);
    return result;
}

static __forceinline u32 be32toh(u32 a) { return read_be32((u8 *)&a); }
static __forceinline u64 htobe64(u64 a) {
    u64 result;
    write_be64((u8 *)&result, a);
    return result;
}

static __forceinline u64 be64toh(u64 a) { return read_be64((u8 *)&a); }
static __forceinline u16 be16add(u16 be, u16 a) {
    return htobe16(be16toh(be) + a);
}

static __forceinline u32 be32add(u32 be, u16 a) {
    return htobe32(be32toh(be) + a);
}

static __forceinline u64 be64add(u64 be, u16 a) {
    return htobe64(be64toh(be) + a);
}

static __forceinline u16 be16sub(u16 be, u16 a) {
    return htobe16(be16toh(be) - a);
}

static __forceinline u32 be32sub(u32 be, u16 a) {
    return htobe32(be32toh(be) - a);
}

static __forceinline u64 be64sub(u64 be, u16 a) {
    return htobe64(be64toh(be) - a);
}

static __forceinline void be16dec(u16 *be) { *be = htobe16(be16toh(*be) - 1); }
static __forceinline void be32dec(u32 *be) { *be = htobe32(be32toh(*be) - 1); }
static __forceinline void be64dec(u64 *be) { *be = htobe64(be64toh(*be) - 1); }
static __forceinline void be16inc(u16 *be) { *be = htobe16(be16toh(*be) + 1); }
static __forceinline void be32inc(u32 *be) { *be = htobe32(be32toh(*be) + 1); }
static __forceinline void be64inc(u64 *be) { *be = htobe64(be64toh(*be) + 1); }
