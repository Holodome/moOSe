#include "kprint.h"

#include "vga.h"

typedef enum {
    TY_INT,
    TY_PTR,
    TY_UINT,
    TY_LONG,
    TY_ULONG,
    TY_LLONG,
    TY_ULLONG,
    TY_CHAR,
    TY_UCHAR,
    TY_SHORT,
    TY_USHORT,
} ty_t;

typedef enum { FMT_REG, FMT_HEX, FMT_OCT, FMT_STR, FMT_PTR, FMT_CHAR } fmt_t;

typedef union {
    size_t i;
    long double f;
    void *p;
} arg_t;

static void get_arg(arg_t *arg, ty_t ty, va_list *args) {
    switch (ty) {
    case TY_PTR:
        arg->p = va_arg(*args, void *);
        break;
    case TY_INT:
        arg->i = va_arg(*args, int);
        break;
    case TY_UINT:
        arg->i = va_arg(*args, unsigned int);
        break;
    case TY_LONG:
        arg->i = va_arg(*args, long);
        break;
    case TY_ULONG:
        arg->i = va_arg(*args, unsigned long);
        break;
    case TY_LLONG:
        arg->i = va_arg(*args, long long);
        break;
    case TY_ULLONG:
        arg->i = va_arg(*args, unsigned long long);
        break;
    case TY_CHAR:
        arg->i = (signed char)va_arg(*args, int);
        break;
    case TY_UCHAR:
        arg->i = (unsigned char)va_arg(*args, int);
        break;
    case TY_SHORT:
        arg->i = (short)va_arg(*args, int);
        break;
    case TY_USHORT:
        arg->i = (unsigned short)va_arg(*args, int);
        break;
    }
}

static void write_constrained(char **dst, char *dst_end, unsigned *lenp,
                              int c) {
    if (*dst < dst_end) {
        **dst = c;
        ++*dst;
    }

    ++*lenp;
}

static int parse_spec(const char **srcp, fmt_t *fmt, ty_t *ty) {
    const char *src = *srcp;
    if (src[0] == 'l' && src[1] == 'l') {
        switch (src[2]) {
        case 'd':
        case 'i':
            *ty = TY_LLONG;
            break;
        case 'u':
            *ty = TY_ULLONG;
            break;
        case 'o':
            *ty = TY_ULLONG;
            *fmt = FMT_OCT;
            break;
        case 'x':
            *ty = TY_ULLONG;
            *fmt = FMT_HEX;
            break;
        default:
            return -1;
        }

        *srcp += 3;
    } else if (src[0] == 'l') {
        switch (src[1]) {
        case 'd':
        case 'i':
            *ty = TY_LONG;
            break;
        case 'u':
            *ty = TY_ULONG;
            break;
        case 'o':
            *ty = TY_ULONG;
            *fmt = FMT_OCT;
            break;
        case 'x':
            *ty = TY_ULONG;
            *fmt = FMT_HEX;
            break;
        default:
            return -1;
        }

        *srcp += 2;
    } else if (src[0] == 'h' && src[1] == 'h') {
        switch (src[2]) {
        case 'd':
        case 'i':
            *ty = TY_CHAR;
            break;
        case 'u':
            *ty = TY_UCHAR;
            break;
        case 'o':
            *ty = TY_UCHAR;
            *fmt = FMT_OCT;
            break;
        case 'x':
            *ty = TY_UCHAR;
            *fmt = FMT_HEX;
            break;
        default:
            return -1;
        }

        *srcp += 3;
    } else if (src[0] == 'h') {
        switch (src[1]) {
        case 'd':
        case 'i':
            *ty = TY_SHORT;
            break;
        case 'u':
            *ty = TY_USHORT;
            break;
        case 'o':
            *ty = TY_USHORT;
            *fmt = FMT_OCT;
            break;
        case 'x':
            *ty = TY_USHORT;
            *fmt = FMT_HEX;
            break;
        default:
            return -1;
        }

        *srcp += 2;
    } else {
        switch (src[0]) {
        case 'p':
            *ty = TY_PTR;
            *fmt = FMT_PTR;
            break;
        case 's':
            *ty = TY_PTR;
            *fmt = FMT_STR;
            break;
        case 'c':
            *fmt = FMT_CHAR;
        case 'd':
        case 'i':
            break;
        case 'u':
            *ty = TY_UINT;
            break;
        case 'o':
            *ty = TY_UINT;
            *fmt = FMT_OCT;
            break;
        case 'x':
            *ty = TY_UINT;
            *fmt = FMT_HEX;
            break;
        default:
            return -1;
        }

        *srcp += 1;
    }

    return 0;
}

static void format_int(char **dstp, char *dst_end, unsigned *lenp, u32 val,
                       fmt_t fmt) {
    static const char hex_digits[] = "0123456789abcdef";

    if (val == 0) {
        write_constrained(dstp, dst_end, lenp, '0');
        return;
    }

    u32 base;
    switch (fmt) {
    case FMT_HEX:
        base = 16;
        break;
    case FMT_OCT:
        base = 8;
        break;
    case FMT_REG:
        base = 10;
        break;
    default:
        __builtin_unreachable();
    }

    u8 temp_buf[CHAR_BIT * sizeof(u32)];
    u32 log = 0;
    for (; val != 0; ++log, val /= base)
        temp_buf[log] = val % base;

    while (--log + 1 != 0)
        write_constrained(dstp, dst_end, lenp, hex_digits[temp_buf[log]]);
}

static void format_str(char **dstp, char *dst_end, unsigned *lenp,
                       const char *str) {
    if (str == NULL)
        str = "(null)";

    while (*str != '\0')
        write_constrained(dstp, dst_end, lenp, *str++);
}

static void format_arg(char **dstp, char *dst_end, unsigned *lenp, arg_t *arg,
                       fmt_t fmt) {
    switch (fmt) {
    case FMT_PTR:
        format_int(dstp, dst_end, lenp, (size_t)arg->p, 16);
        break;
    case FMT_STR:
        format_str(dstp, dst_end, lenp, arg->p);
        break;
    case FMT_REG:
        if (arg->i > I32_MAX) {
            arg->i = -arg->i;
            write_constrained(dstp, dst_end, lenp, '-');
        }
        // fallthrough
    case FMT_OCT:
    case FMT_HEX:
        format_int(dstp, dst_end, lenp, arg->i, fmt);
        break;
    case FMT_CHAR:
        write_constrained(dstp, dst_end, lenp, arg->i);
        break;
    }
}

int vsnprintf(char *dst, size_t dst_size, const char *src, va_list lst) {
    if (dst_size != 0)
        *dst = '\0';

    char *dst_end = dst + dst_size;
    unsigned len = 0;
    for (;;) {
        while (*src != '%' && *src != '\0')
            write_constrained(&dst, dst_end, &len, *src++);

        if (*src++ == '\0')
            break;

        if (*src == '%') {
            write_constrained(&dst, dst_end, &len, *src++);
            continue;
        }

        fmt_t fmt = FMT_REG;
        ty_t ty = TY_INT;
        int parse_res = parse_spec(&src, &fmt, &ty);
        if (parse_res != 0)
            return -1;

        arg_t arg;
        get_arg(&arg, ty, &lst);
        format_arg(&dst, dst_end, &len, &arg, fmt);
    }

    if (len >= dst_size && dst_size != 0)
        dst_end[-1] = '\0';
    else if (dst_size != 0)
        *dst = '\0';

    return len;
}

int snprintf(char *dst, size_t dst_size, const char *src, ...) {
    va_list lst;
    va_start(lst, src);
    int result = vsnprintf(dst, dst_size, src, lst);
    va_end(lst);
    return result;
}

int kvprintf(const char *fmt, va_list lst) {
    char buffer[256];
    int result = vsnprintf(buffer, sizeof(buffer), fmt, lst);
    kputs(buffer);
    return result;
}

int kprintf(const char *fmt, ...) {
    va_list lst;
    va_start(lst, fmt);
    int result = kvprintf(fmt, lst);
    va_end(lst);
    return result;
}

