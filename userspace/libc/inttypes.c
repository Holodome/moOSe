#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

imaxdiv_t imaxdiv(intmax_t x, intmax_t y) {
    imaxdiv_t result = {.quot = x / y, .rem = x % y};
    if (x > 0 && result.rem < 0) {
        ++result.quot;
        result.rem -= y;
    }

    return result;
}

intmax_t strtoimax(const char *start, char **endptr, int base) {
    static_assert(sizeof(intmax_t) == sizeof(long long), "not implemented");
    return strtoll(start, endptr, base);
}

uintmax_t strtoumax(const char *start, char **endptr, int base) {
    static_assert(sizeof(intmax_t) == sizeof(long long), "not implemented");
    return strtoull(start, endptr, base);
}
