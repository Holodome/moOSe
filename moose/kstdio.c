#include "kstdio.h"
#include "console_display.h"

#define NUMBER_BUFFER_SIZE 32

static void reverse_buffer(char *buffer, size_t size) {
    for (size_t i = 0; i < size / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[size - i - 1];
        buffer[size - i - 1] = temp;
    }
}

static void copy_buffer(char *dst, const char *src, size_t dst_size, size_t *counter, size_t src_size) {
    for (size_t i = 0; i < src_size; i++) {
        if (*counter < dst_size)
            dst[*counter] = src[i];
        *counter = *counter + 1;
    }
}

static size_t convert_number(char *buffer, u64 number, int base) {
    size_t counter = 0;
    char *charset = "0123456789abcdef";
    if (number == 0)
        buffer[counter++] = '0';

    while (number > 0) {
        int digit = number % base;
        buffer[counter++] = charset[digit];
        number /= base;
    }

    return counter;
}

static void print_minus(char *buffer, size_t size, size_t *counter) {
    if (*counter < size)
        buffer[*counter] = '-';
    *counter = *counter + 1;
}

static void print_signed(char *buffer, size_t buffer_size, size_t *counter, u64 number, int base) {
    if (number < 0) {
        print_minus(buffer, buffer_size, counter);
        number *= -1;
    }

    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t printed = convert_number(number_buffer, number, base);
    reverse_buffer(number_buffer, printed);
    copy_buffer(buffer, number_buffer, buffer_size, counter, printed);
}

static void print_unsigned(char *buffer, size_t buffer_size, size_t *counter, u64 number, int base) {
    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t printed = convert_number(number_buffer, number, base);
    reverse_buffer(number_buffer, printed);
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

int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args) {
    size_t counter = 0;
    
    while (*fmt) {
        while (*fmt && *fmt != '%') {
            if (counter < size) {
                buffer[counter] = *fmt++;
                counter++;
            } else {
                counter++;
                fmt++;
            }
        }
        
        if (*fmt == '\0')
            break;

        fmt++;

        switch (*fmt) {
        case 'h':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, (i64) va_arg(args, i32), 10);
                break;
            case 'o':
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), 8);
                break;
            case 'x':
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), 16);
                break;
            }
            break;
        case 'l':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, (i64) va_arg(args, i64), 10);
                break;
            case 'o':
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u64), 8);
                break;
            case 'x':
                print_unsigned(buffer, size, &counter, (u64) va_arg(args, u64), 16);
                break;
            }
            break;
        case 'i':
        case 'd':
            print_signed(buffer, size, &counter, (i64) va_arg(args, i32), 10);
            break;
        case 'o':
            print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), 8);
            break;
        case 'x':
            print_unsigned(buffer, size, &counter, (u64) va_arg(args, u32), 16);
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

    va_end(args);

    return (int) counter;
}

int snprintf(char *buffer, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(buffer, size, fmt, args);

    va_end(args);

    return count;
}
