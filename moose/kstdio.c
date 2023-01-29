#include "kstdio.h"
#include "console_display.h"
#include "kmem.h"

typedef struct {
    size_t length;
    u8 is_left_align;
    char padding_char;
    u8 is_plus_char;
    u8 is_hex_fmt;
    u8 is_hex_upper;
    u8 base;
} printf_opts_t;

int snprintf(char *buffer, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(buffer, size, fmt, args);

    va_end(args);

    return count;
}

#define NUMBER_BUFFER_SIZE 32

static void copy_buffer(char *dst, const char *src, size_t dst_size, size_t *counter, size_t src_size) {
    for (size_t i = 0; i < src_size; i++) {
        if (*counter < dst_size)
            dst[*counter] = src[i];
        *counter = *counter + 1;
    }
}

static size_t print_number(char *buffer, u64 number, int base) {
    size_t counter = 0;
    char *charset = "0123456789abcdef";
    if (number == 0)
        buffer[counter++] = '0';

    while (number > 0) {
        int digit = number % base;
        buffer[counter++] = charset[digit];
        number /= base;
    }

    for (size_t i = 0; i < counter / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[counter - i - 1];
        buffer[counter - i - 1] = temp;
    }

    return counter;
}

static void print_signed(char *buffer, size_t size, size_t *counter,
                         i64 number, printf_opts_t opts) {
    u8 is_negative = number < 0;
    if (is_negative)
        number *= -1;

    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t number_length = print_number(number_buffer, number, 10);

    if (opts.length >= number_length)
        opts.length -= number_length;
    else opts.length = 0;

    if (opts.padding_char == ' ' && !opts.is_left_align) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = ' ';
            *counter = *counter + 1;
        }
    }

    if (is_negative) {
        if (*counter < size)
            buffer[*counter] = '-';
        *counter = *counter + 1;
    } else if (opts.is_plus_char) {
        if (*counter < size)
            buffer[*counter] = '+';
        *counter = *counter + 1;
    }

    if (opts.padding_char == '0' && !opts.is_left_align) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = '0';
            *counter = *counter + 1;
        }
    }

    for (size_t i = 0; i < number_length; i++) {
        if (*counter < size)
            buffer[*counter] = number_buffer[i];
        *counter = *counter + 1;
    }

    if (opts.is_left_align) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = ' ';
            *counter = *counter + 1;
        }
    }
}

static void print_unsigned(char *buffer, size_t buffer_size, size_t *counter,
                           u64 number, printf_opts_t opts) {
    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t printed = print_number(number_buffer, number, opts.base);
    copy_buffer(buffer, number_buffer, buffer_size, counter, printed);
}

static void print_string(char *buffer, size_t buffer_size, size_t *counter, char *str) {
    while (*str) {
        if (*counter < buffer_size)
            buffer[*counter] = *str;
        *counter = *counter + 1;
        str++;
    }
}

static u8 isdigit(char c) {
    return c >= '0' && c <= '9';
}

int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args) {
    size_t counter = 0;
    
    while (*fmt) {
        if (*fmt != '%') {
            if (counter < size) {
                buffer[counter] = *fmt++;
                counter++;
            } else {
                counter++;
                fmt++;
            }
            continue;
        }

        fmt++;

        printf_opts_t opts;
        memset(&opts, 0, sizeof(printf_opts_t));
        opts.padding_char = ' ';

        for (;;) {
            char c = *fmt;
            if (c == '+')
                opts.is_plus_char = 1;
            else if (c == '-')
                opts.is_left_align = 1;
            else if (c == '#')
                opts.is_hex_fmt = 1;
            else break;
            fmt++;
        }

        if (*fmt == '0') {
            opts.padding_char = '0';
            fmt++;
        }

        while (isdigit(*fmt)) {
            opts.length = 10 * opts.length + (*fmt - '0');
            fmt++;
        }

        switch (*fmt) {
        case 'h':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                opts.base = 10;
                print_signed(buffer, size, &counter, (i64) va_arg(args, i32), opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), opts);
                break;
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), opts);
                break;
            }
            break;
        case 'l':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                opts.base = 10;
                print_signed(buffer, size, &counter, (i64) va_arg(args, i64), opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u64), opts);
                break;
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u64), opts);
                break;
            }
            break;
        case 'i':
        case 'd':
            opts.base = 10;
            print_signed(buffer, size, &counter, (i64) va_arg(args, i32), opts);
            break;
        case 'o':
            opts.base = 8;
            print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), opts);
            break;
        case 'x':
            opts.base = 16;
            print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), opts);
            break;
        case 'X':
            opts.base = 16;
            opts.is_hex_upper = 1;
            print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), opts);
            break;
        case 'c':
            if (counter < size)
                buffer[counter] = (char) va_arg(args, int);
            counter++;
            break;
        case 's':
            print_string(buffer, size, &counter, (char *) va_arg(args, char *));
            break;
        default:
            if (counter < size)
                buffer[counter] = *fmt;
            counter++;
            break;
        }
        fmt++;
    }

    if (counter < size && size)
        buffer[counter] = '\0';
    else if (counter >= size && size)
        buffer[size - 1] = '\0';

    return (int) counter;
}

int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = kvprintf(fmt, args);

    va_end(args);

    return count;
}

int kvprintf(const char *fmt, va_list args) {
    char buffer[256];
    int count = vsnprintf(buffer, 256, fmt, args);

    size_t len = strlen(buffer);
    console_print(buffer, len);

    return count;
}

int kputc(int c) {
    console_print((char *) &c, 1);
    return 1;
}

int kputs(const char *str) {
    size_t len = strlen(str);
    console_print(str, len);
    kputc('\n');
    return len;
}
