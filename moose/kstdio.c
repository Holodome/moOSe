#include <tty.h>
#include <kmem.h>
#include <kstdio.h>

struct printf_opts {
    size_t length;
    char padding_char;
    int is_left_aligned;
    int is_plus_char;
    int is_hex_fmt;
    int is_hex_uppercase;
    int base;
};

static int isdigit(char c) { return c >= '0' && c <= '9'; }

int snprintf(char *buffer, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = vsnprintf(buffer, size, fmt, args);

    va_end(args);

    return count;
}

#define NUMBER_BUFFER_SIZE 32

static size_t print_number(char *buffer, unsigned long long number, int base) {
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
                         long long number, struct printf_opts opts) {
    int is_negative = number < 0;
    if (is_negative)
        number *= -1;

    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t number_length = print_number(number_buffer, number, 10);

    if (opts.length >= number_length)
        opts.length -= number_length;
    else
        opts.length = 0;

    if (opts.padding_char == ' ' && !opts.is_left_aligned) {
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

    if (opts.padding_char == '0' && !opts.is_left_aligned) {
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

    if (opts.is_left_aligned) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = ' ';
            *counter = *counter + 1;
        }
    }
}

static void print_unsigned(char *buffer, size_t size, size_t *counter,
                           unsigned long long number, struct printf_opts opts) {
    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t number_length = print_number(number_buffer, number, opts.base);

    char *number_prefix = "";
    size_t prefix_length = 0;

    if (opts.is_hex_fmt) {
        if (opts.base == 8)
            number_prefix = "0";
        else if (opts.base == 16 && opts.is_hex_uppercase)
            number_prefix = "0X";
        else if (opts.base == 16)
            number_prefix = "0x";
        prefix_length = strlen(number_prefix);
    }

    if (opts.length >= (number_length + prefix_length))
        opts.length -= (number_length + prefix_length);
    else
        opts.length = 0;

    if (opts.padding_char == ' ' && !opts.is_left_aligned) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = opts.padding_char;
            *counter = *counter + 1;
        }
    }

    if (opts.is_hex_fmt) {
        for (size_t i = 0; i < prefix_length; i++) {
            if (*counter < size)
                buffer[*counter] = number_prefix[i];
            *counter = *counter + 1;
        }
    }

    if (opts.padding_char == '0' && !opts.is_left_aligned) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = '0';
            *counter = *counter + 1;
        }
    }

    for (size_t i = 0; i < number_length; i++) {
        if (*counter < size) {
            if (opts.is_hex_uppercase && !isdigit(number_buffer[i]))
                buffer[*counter] = number_buffer[i] - ('a' - 'A');
            else
                buffer[*counter] = number_buffer[i];
        }
        *counter = *counter + 1;
    }

    if (opts.is_left_aligned) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = ' ';
            *counter = *counter + 1;
        }
    }
}

static void print_string(char *buffer, size_t size, size_t *counter, char *str,
                         struct printf_opts opts) {
    size_t length = strlen(str);
    if (opts.length >= length)
        opts.length -= length;
    else
        opts.length = 0;

    if (!opts.is_left_aligned) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = ' ';
            *counter = *counter + 1;
        }
    }

    while (*str) {
        if (*counter < size)
            buffer[*counter] = *str;
        *counter = *counter + 1;
        str++;
    }

    if (opts.is_left_aligned) {
        while (opts.length--) {
            if (*counter < size)
                buffer[*counter] = ' ';
            *counter = *counter + 1;
        }
    }
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

        struct printf_opts opts = {0};
        opts.padding_char = ' ';
        opts.base = 10;

        for (;;) {
            char c = *fmt;
            if (c == '+')
                opts.is_plus_char = 1;
            else if (c == '-')
                opts.is_left_aligned = 1;
            else if (c == '#')
                opts.is_hex_fmt = 1;
            else
                break;
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
        case 't':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
            case 'u':
                print_signed(buffer, size, &counter, va_arg(args, ptrdiff_t),
                             opts);
                break;
            case 'o':
                opts.base = 8;
                print_signed(buffer, size, &counter, va_arg(args, ptrdiff_t),
                             opts);
                break;
            case 'X':
                opts.base = 16;
                print_signed(buffer, size, &counter, va_arg(args, ptrdiff_t),
                             opts);
                break;
            }
            break;
        case 'j':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, va_arg(args, intmax_t),
                             opts);
                break;
            case 'u':
                print_unsigned(buffer, size, &counter, va_arg(args, uintmax_t),
                               opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter, va_arg(args, uintmax_t),
                               opts);
                break;
            case 'X':
                opts.base = 16;
                print_unsigned(buffer, size, &counter, va_arg(args, uintmax_t),
                               opts);
                break;
            }
            break;
        case 'z':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
            case 'u':
                print_unsigned(buffer, size, &counter, va_arg(args, size_t),
                               opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter, va_arg(args, size_t),
                               opts);
                break;
            case 'X':
                opts.base = 16;
                print_unsigned(buffer, size, &counter, va_arg(args, size_t),
                               opts);
                break;
            }
            break;
        case 'h':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, va_arg(args, int), opts);
                break;
            case 'u':
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned int), opts);
                break;
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned int), opts);
                break;
            case 'h':
                fmt++;
                switch (*fmt) {
                case 'i':
                case 'd':
                    print_signed(buffer, size, &counter, va_arg(args, int),
                                 opts);
                    break;
                case 'o':
                    opts.base = 8;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned int), opts);
                    break;
                case 'u':
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned int), opts);
                    break;
                case 'x':
                    opts.base = 16;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned int), opts);
                    break;
                }
            }
            break;
        case 'l':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, va_arg(args, long int),
                             opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned long int), opts);
                break;
            case 'u':
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned long int), opts);
                break;
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned long int), opts);
                break;
            case 'l':
                fmt++;
                switch (*fmt) {
                case 'i':
                case 'd':
                    print_signed(buffer, size, &counter,
                                 va_arg(args, long long int), opts);
                    break;
                case 'o':
                    opts.base = 8;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned long long int), opts);
                    break;
                case 'u':
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned long long int), opts);
                    break;
                case 'x':
                    opts.base = 16;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned long long int), opts);
                    break;
                }
            }
            break;
        case 'i':
        case 'd':
            print_signed(buffer, size, &counter, va_arg(args, int), opts);
            break;
        case 'o':
            opts.base = 8;
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           opts);
            break;
        case 'u':
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           opts);
            break;
        case 'x':
            opts.base = 16;
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           opts);
            break;
        case 'X':
            opts.base = 16;
            opts.is_hex_uppercase = 1;
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           opts);
            break;
        case 'c':
            if (counter < size)
                buffer[counter] = (char)va_arg(args, int);
            counter++;
            break;
        case 's':
            print_string(buffer, size, &counter, (char *)va_arg(args, char *),
                         opts);
            break;
        case 'p':
            opts.base = 16;
            opts.is_hex_fmt = 1;
            print_unsigned(buffer, size, &counter,
                           (unsigned long long int)va_arg(args, void *), opts);
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

    return (int)counter;
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
    int write_result = tty_write(buffer, strlen(buffer));
    return write_result < 0 ? write_result : count;
}

int kputc(int c) {
    return tty_write((char *)&c, 1);
}

int kputs(const char *str) {
    size_t len = strlen(str);
    int result = tty_write(str, len);
    if (result == -1) 
        return result;

    kputc('\n');
    return (int)len;
}
