#pragma once

#ifndef __i686__

typedef signed char i8;
typedef unsigned char u8;
typedef signed short i16;
typedef unsigned short u16;
typedef signed int i32;
typedef unsigned int u32;
typedef signed long int i64;
typedef unsigned long int u64;

typedef unsigned long int size_t;
typedef unsigned long int uintptr_t;
typedef signed long int ssize_t;
typedef signed long int ptrdiff_t;
typedef signed long int intmax_t;
typedef unsigned long int uintmax_t;
typedef signed long int intptr_t;

#define SIZE_MAX ((size_t)ULONG_MAX)
#define UINTPTR_MAX ((uintptr_t)ULONG_MAX)
#define SSIZE_MAX ((ssize_t)LONG_MAX)
#define PTRDIFF_MIN ((ptrdiff_t)LONG_MIN)
#define PTRDIFF_MAX ((ptrdiff_t)LONG_MAX)
#define INTMAX_MAX ((intmax_t)LONG_MAX)
#define INTMAX_MIN ((intmax_t)LONG_MIN)

#else

typedef signed char i8;
typedef unsigned char u8;
typedef signed short i16;
typedef unsigned short u16;
typedef signed int i32;
typedef unsigned int u32;
typedef signed long long int i64;
typedef unsigned long long int u64;

typedef unsigned int size_t;
typedef unsigned int uintptr_t;
typedef signed int intptr_t;
typedef signed int ssize_t;
typedef signed int ptrdiff_t;
typedef signed long int intmax_t;
typedef unsigned long int uintmax_t;

#define SIZE_MAX ((size_t)UINT_MAX)
#define UINTPTR_MAX ((uintptr_t)UINT_MAX)
#define SSIZE_MAX ((ssize_t)INT_MAX)
#define PTRDIFF_MIN ((ptrdiff_t)INT_MIN)
#define PTRDIFF_MAX ((ptrdiff_t)INT_MAX)
#define INTMAX_MAX ((intmax_t)INT_MAX)
#define INTMAX_MIN ((intmax_t)INT_MIN)

#endif

#define CHAR_BIT 8
#define WORD_BIT ((sizeof(int) * CHAR_BIT)
#define LONG_BIT ((sizeof(long) * CHAR_BIT)
#define SIZE_WIDTH ((sizeof(size_t) * CHAR_BIT)

#define CHAR_MAX ((char)0x7f)
#define CHAR_MIN ((char)0x80)
#define SHRT_MAX ((short)0x7fff)
#define SHRT_MIN ((short)0x8000)
#define INT_MAX ((int)0x7fffffff)
#define INT_MIN ((int)0x80000000)
#define LONG_MAX ((long)0x7fffffffffffffff)
#define LONG_MIN ((long)0x8000000000000000)
#define LLONG_MAX ((long long)LONG_MAX)
#define LLONG_MIN ((long long)LONG_MIN)

#define UCHAR_MAX ((unsigned char)0xff)
#define SCHAR_MAX ((signed char)CHAR_MAX)
#define SCHAR_MIN ((signed char)CHAR_MIN)
#define USHRT_MAX ((unsigned short)0xffff)
#define UINT_MAX ((unsigned int)0xffffffff)
#define ULONG_MAX ((unsigned long)0xffffffffffff)
#define ULLONG_MAX ((unsigned long)ULONG_MAX)

#define NULL ((void *)0)
