#include <moose/assert.h>
#include <moose/ctype.h>
#include <moose/kstdio.h>
#include <moose/string.h>
#include <moose/tty/vga_console.h>
#include <moose/tty/vterm.h>

struct printf_opts {
    size_t width;
    char padding_char;
    int base;
    int precision;
    unsigned is_left_aligned : 1;
    unsigned is_plus_char : 1;
    unsigned is_hex_uppercase : 1;
    unsigned has_precision : 1;
    unsigned force_prefix : 1;
};

#define outc(_buffer, _size, _counter, _c)                                     \
    do {                                                                       \
        if (*(_counter) < (_size))                                             \
            (_buffer)[*(_counter)] = (_c);                                     \
        *(_counter) += 1;                                                      \
    } while (0)

int snprintf(char *buffer, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = vsnprintf(buffer, size, fmt, args);
    va_end(args);

    return count;
}

#define NUMBER_BUFFER_SIZE (CHAR_BIT * sizeof(uintmax_t))

static size_t print_number(char *buffer, uintmax_t number, int base) {
    size_t counter = 0;
    if (number == 0)
        buffer[counter++] = '0';

    while (number > 0) {
        int digit = number % base;
        buffer[counter++] = "0123456789acbdef"[digit];
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
                         intmax_t number, struct printf_opts *opts) {
    int is_negative = number < 0;
    if (is_negative)
        number *= -1;

    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t number_length = print_number(number_buffer, number, 10);
    expects(number_length < sizeof(number_buffer));

    if (opts->width >= number_length)
        opts->width -= number_length;
    else
        opts->width = 0;

    if (!opts->is_left_aligned && opts->padding_char != '0') {
        while (opts->width--)
            outc(buffer, size, counter, opts->padding_char);
    }

    if (is_negative)
        outc(buffer, size, counter, '-');
    else if (opts->is_plus_char)
        outc(buffer, size, counter, '+');

    if (!opts->is_left_aligned && opts->padding_char == '0') {
        while (opts->width--)
            outc(buffer, size, counter, opts->padding_char);
    }

    for (size_t i = 0; i < number_length; i++)
        outc(buffer, size, counter, number_buffer[i]);

    if (opts->is_left_aligned) {
        while (opts->width--)
            outc(buffer, size, counter, ' ');
    }
}

static void print_unsigned(char *buffer, size_t size, size_t *counter,
                           uintmax_t number, struct printf_opts *opts) {
    char number_buffer[NUMBER_BUFFER_SIZE];
    size_t number_length = print_number(number_buffer, number, opts->base);

    const char *prefix = "";
    size_t prefix_length = 0;
    if (opts->force_prefix) {
        if (opts->base == 8)
            prefix = "0";
        else if (opts->base == 16 && opts->is_hex_uppercase)
            prefix = "0X";
        else if (opts->base == 16)
            prefix = "0x";
        prefix_length = strlen(prefix);
    }

    if (opts->width >= (number_length + prefix_length))
        opts->width -= (number_length + prefix_length);
    else
        opts->width = 0;

    if (!opts->is_left_aligned && opts->padding_char != '0') {
        while (opts->width--)
            outc(buffer, size, counter, opts->padding_char);
    }

    for (size_t i = 0; i < prefix_length; i++)
        outc(buffer, size, counter, prefix[i]);

    if (!opts->is_left_aligned && opts->padding_char == '0') {
        while (opts->width--)
            outc(buffer, size, counter, opts->padding_char);
    }

    for (size_t i = 0; i < number_length; i++) {
        int c = number_buffer[i];
        if (opts->is_hex_uppercase)
            c = toupper(c);
        outc(buffer, size, counter, c);
    }

    if (opts->is_left_aligned) {
        while (opts->width--)
            outc(buffer, size, counter, ' ');
    }
}

static void print_string(char *buffer, size_t size, size_t *counter, char *str,
                         struct printf_opts *opts) {
    size_t length = strlen(str);
    if (opts->width >= length)
        opts->width -= length;
    else
        opts->width = 0;

    if (!opts->is_left_aligned) {
        while (opts->width--)
            outc(buffer, size, counter, ' ');
    }

    int max_length = INT_MAX;
    if (opts->has_precision)
        max_length = opts->precision;

    while (*str && max_length--)
        outc(buffer, size, counter, *str++);

    if (opts->is_left_aligned) {
        while (opts->width--)
            outc(buffer, size, counter, ' ');
    }
}

int vsnprintf(char *buffer, size_t size, const char *fmt, va_list args) {
    size_t counter = 0;

    while (*fmt) {
        while (*fmt != '%' && *fmt)
            outc(buffer, size, &counter, *fmt++);

        if (!*fmt)
            break;

        // skip %
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
                opts.force_prefix = 1;
            else
                break;
            fmt++;
        }

        if (*fmt == '0') {
            opts.padding_char = '0';
            fmt++;
        }

        // width
        if (isdigit(*fmt)) {
            do {
                opts.width = 10 * opts.width + (*fmt - '0');
                fmt++;
            } while (isdigit(*fmt));
        } else if (*fmt == '*') {
            opts.width = va_arg(args, int);
            fmt++;
        }

        // precision
        if (*fmt == '.') {
            opts.has_precision = 1;
            fmt++;
            if (isdigit(*fmt)) {
                do {
                    opts.precision = 10 * opts.precision + (*fmt - '0');
                    fmt++;
                } while (isdigit(*fmt));
            } else if (*fmt == '*') {
                opts.precision = va_arg(args, int);
                fmt++;
            }
        }

        switch (*fmt) {
        case 't':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
            case 'u':
                print_signed(buffer, size, &counter, va_arg(args, ptrdiff_t),
                             &opts);
                break;
            case 'o':
                opts.base = 8;
                print_signed(buffer, size, &counter, va_arg(args, ptrdiff_t),
                             &opts);
                break;
            case 'X':
                opts.is_hex_uppercase = 1;
                // fallthrough
            case 'x':
                opts.base = 16;
                print_signed(buffer, size, &counter, va_arg(args, ptrdiff_t),
                             &opts);
                break;
            }
            break;
        case 'j':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, va_arg(args, intmax_t),
                             &opts);
                break;
            case 'u':
                print_unsigned(buffer, size, &counter, va_arg(args, uintmax_t),
                               &opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter, va_arg(args, uintmax_t),
                               &opts);
                break;
            case 'X':
                opts.is_hex_uppercase = 1;
                // fallthrough
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter, va_arg(args, uintmax_t),
                               &opts);
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
                               &opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter, va_arg(args, size_t),
                               &opts);
                break;
            case 'X':
                opts.is_hex_uppercase = 1;
                // fallthrough
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter, va_arg(args, size_t),
                               &opts);
                break;
            }
            break;
        case 'h':
            fmt++;
            switch (*fmt) {
            case 'i':
            case 'd':
                print_signed(buffer, size, &counter, va_arg(args, int), &opts);
                break;
            case 'u':
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned int), &opts);
                break;
            case 'X':
                opts.is_hex_uppercase = 1;
                // fallthrough
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned int), &opts);
                break;
            case 'h':
                fmt++;
                switch (*fmt) {
                case 'i':
                case 'd':
                    print_signed(buffer, size, &counter, va_arg(args, int),
                                 &opts);
                    break;
                case 'o':
                    opts.base = 8;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned int), &opts);
                    break;
                case 'u':
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned int), &opts);
                    break;
                case 'X':
                    opts.is_hex_uppercase = 1;
                    // fallthrough
                case 'x':
                    opts.base = 16;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned int), &opts);
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
                             &opts);
                break;
            case 'o':
                opts.base = 8;
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned long int), &opts);
                break;
            case 'u':
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned long int), &opts);
                break;
            case 'X':
                opts.is_hex_uppercase = 1;
                // fallthrough
            case 'x':
                opts.base = 16;
                print_unsigned(buffer, size, &counter,
                               va_arg(args, unsigned long int), &opts);
                break;
            case 'l':
                fmt++;
                switch (*fmt) {
                case 'i':
                case 'd':
                    print_signed(buffer, size, &counter,
                                 va_arg(args, long long int), &opts);
                    break;
                case 'o':
                    opts.base = 8;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned long long int), &opts);
                    break;
                case 'u':
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned long long int), &opts);
                    break;
                case 'X':
                    opts.is_hex_uppercase = 1;
                    // fallthrough
                case 'x':
                    opts.base = 16;
                    print_unsigned(buffer, size, &counter,
                                   va_arg(args, unsigned long long int), &opts);
                    break;
                }
            }
            break;
        case 'i':
        case 'd':
            print_signed(buffer, size, &counter, va_arg(args, int), &opts);
            break;
        case 'o':
            opts.base = 8;
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           &opts);
            break;
        case 'u':
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           &opts);
            break;
        case 'X':
            opts.is_hex_uppercase = 1;
            // fallthrough
        case 'x':
            opts.base = 16;
            print_unsigned(buffer, size, &counter, va_arg(args, unsigned int),
                           &opts);
            break;
        case 'c':
            if (counter < size)
                buffer[counter] = (char)va_arg(args, int);
            counter++;
            break;
        case 's':
            print_string(buffer, size, &counter, (char *)va_arg(args, char *),
                         &opts);
            break;
        case 'p':
            opts.base = 16;
            opts.force_prefix = 1;
            print_unsigned(buffer, size, &counter,
                           (uintptr_t)va_arg(args, void *), &opts);
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

static struct {
    int is_initialized;
    struct vterm *term;
} kstdio_state;

int kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = kvprintf(fmt, args);
    va_end(args);

    return count;
}

int kvprintf(const char *fmt, va_list args) {
    if (!kstdio_state.is_initialized)
        return 0;

    char buffer[256];
    int count = vsnprintf(buffer, sizeof(buffer), fmt, args);
    kprint(buffer, strlen(buffer));
    return count;
}

void kprint(const char *str, size_t count) {
    vterm_write(kstdio_state.term, str, count);
}

int init_kstdio(void) {
    struct console *console = create_empty_console();
    if (!console)
        return -1;

    if (vga_init_console(console)) {
        console_release(console);
        return -1;
    }

    struct vterm *term = create_vterm(console);
    if (!term) {
        console_release(console);
        return -1;
    }

    kstdio_state.is_initialized = 1;
    kstdio_state.term = term;

    return 0;
}
