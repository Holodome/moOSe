#pragma once

// FIXME: Adjust architectures
#include "arch/types.h"

#define CHECK_TYPE(_type, _size) \
    _Static_assert(sizeof(_type) == _size, "Type " #_type " size does not equal " #_size)

CHECK_TYPE(i8, 1);
CHECK_TYPE(u8, 1);
CHECK_TYPE(i16, 2);
CHECK_TYPE(u16, 2);
CHECK_TYPE(i32, 4);
CHECK_TYPE(u32, 4);
CHECK_TYPE(i64, 8);
CHECK_TYPE(u64, 8);

#undef CHECK_TYPE

#define I8_MAX ((i8)0x7F)
#define U8_MAX ((u8)0xFF)
#define I16_MAX ((i16)0x7FFF)
#define U16_MAX ((u16)0xFFFF)
#define I32_MAX ((i32)0x7FFFFFFF)
#define U32_MAX ((u32)0xFFFFFFFF)
#define I64_MAX ((i64)0x7FFFFFFFFFFFFFFF)
#define U64_MAX ((u64)0xFFFFFFFFFFFFFFFF)

#define NULL ((void *)0)

typedef __builtin_va_list va_list;

#define va_start(_list, _arg) __builtin_va_start(_list, _arg)
#define va_arg(_list, _type) __builtin_va_arg(_list, _type)
#define va_end(_list) __builtin_va_end(_list)
