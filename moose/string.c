#include <string.h>

char *strpbrk(const char *string, const char *lookup) {
    char *cursor = (char *)string;
    char symb;

    while ((symb = *cursor++))
        for (const char *test = lookup; *test; ++test)
            if (*test == symb) return cursor - 1;

    return NULL;
}

size_t strspn(const char *string, const char *lookup) {
    const char *cursor = string;
    char symb;
    int is_valid = 1;

    while ((symb = *cursor++) && is_valid) {
        is_valid = 0;

        for (const char *test = lookup; *test && !is_valid; ++test)
            is_valid = *test == symb;

        if (!is_valid) --cursor;
    }

    return cursor - string - 1;
}

size_t strcspn(const char *string, const char *lookup) {
    const char *cursor = string;
    int is_valid = 1;
    char symb;

    while ((symb = *cursor++) && is_valid)
        for (const char *test = lookup; *test && is_valid; ++test)
            if (symb == *test) {
                --cursor;
                is_valid = 0;
            }

    return cursor - string - 1;
}

char *strchr(const char *string, int symb) {
    do {
        if (*string == symb) return (char *)string;
    } while (*string++);

    return NULL;
}

char *strrchr(const char *string, int symb) {
    const char *result = NULL;

    do {
        if (*string == symb) result = string;
    } while (*string++);

    return (char *)result;
}

size_t strlcpy(char *dst, const char *src, size_t maxlen) {
    const size_t srclen = strlen(src);
    if (srclen + 1 < maxlen) {
        memcpy(dst, src, srclen + 1);
    } else if (maxlen != 0) {
        memcpy(dst, src, maxlen - 1);
        dst[maxlen - 1] = '\0';
    }
    return srclen;
}

void *memcpy(void *dst_, const void *src_, size_t c) {
    u8 *dst = dst_;
    const u8 *src = src_;
    while (c--) *dst++ = *src++;
    return dst_;
}

void *memset(void *dst_, int ch, size_t c) {
    u8 *dst = dst_;
    while (c--) *dst++ = ch;
    return dst_;
}

void *memmove(void *dst_, const void *src_, size_t c) {
    u8 *dst = dst_;
    const u8 *src = src_;
    if (dst == src) return dst;

    if (dst < src) {
        for (; c; --c) *dst++ = *src++;
    } else {
        while (c) {
            --c;
            dst[c] = src[c];
        }
    }

    return dst_;
}

int memcmp(const void *l_, const void *r_, size_t c) {
    const u8 *l = l_;
    const u8 *r = r_;
    while (c--) {
        int d = *l++ - *r++;
        if (d) return d;
    }

    return 0;
}

size_t strlen(const char *str) {
    const char *cur = str;
    while (*++cur) {}
    return cur - str;
}

char *strncpy(char *dst, const char *src, size_t c) {
    char *ptr = dst;
    while (*src && c--) *dst++ = *src++;
    *dst = '\0';
    return ptr;
}

char *strcpy(char *dst, const char *src) {
    void *result = dst;
    int c;
    while ((c = *src++)) *dst++ = c;
    *dst = '\0';
    return result;
}

char *strcat(char *dst, const char *src) {
    void *result = dst;
    while (*dst++) {}
    strcpy(dst, src);
    return result;
}

char *strncat(char *dst, const char *src, size_t c) {
    void *result = dst;
    while (*dst++) {}
    strncpy(dst, src, c);
    return result;
}

size_t strlcat(char *dst, const char *src, size_t size) {
    char *d = dst;
    const char *s = src;
    size_t n = size;

    while (n-- && *d) d++;

    size_t dlen = d - dst;
    n = size - dlen;

    if (n == 0) return dlen + strlen(s);
    while (*s) {
        if (n != 1) {
            *d++ = *s;
            n--;
        }
        s++;
    }
    *d = '\0';

    return dlen + (s - src);
}

int strcmp(const char *as, const char *bs) {
    int a, b;
    while ((a = *as++) && (b = *bs++))
        if (a != b) return a - b;

    return 0;
}

int strncmp(const char *as, const char *bs, size_t c) {
    int a, b;
    while ((a = *as++) && (b = *bs++) && c--)
        if (a != b) return a - b;

    return 0;
}

char *strchrnul(const char *s, int ch) {
    do {
        if (*s == ch) return (char *)s;
    } while (*s++);

    return (char *)s;
}

char *strnchr(const char *s, int ch, size_t c) {
    if (!c) return NULL;

    do {
        if (*s == ch) return (char *)s;
    } while (*s++ && --c);

    return NULL;
}

char *strnchrnul(const char *s, int ch, size_t c) {
    if (!c) return (char *)s;

    do {
        if (*s == ch) return (char *)s;
    } while (*s++ && --c);

    return (char *)s;
}

char *strstr(const char *s, const char *a) {
    if (!*a) return (char *)s;
    for (; *s; ++s) {
        if (*s != *a) continue;

        const char *test_s = s;
        const char *test_b = s;
        for (;;) {
            if (!*test_b) return (char *)s;
            if (*test_s++ != *test_b++) break;
        }
    }

    return NULL;
}

size_t strnlen(const char *s, size_t c) {
    if (c == 0) return 0;

    const char *start = s;
    while (*s++ && c--) {}
    return s - start;
}

char *strsep(char **sp, const char *sep) {
    char *start = *sp;
    char *found = (void *)strpbrk(start, sep);
    if (found == NULL) return NULL;

    *sp = found + 1;
    *found = '\0';
    return start;
}

void *memchr(const void *src_, int ch, size_t c) {
    const u8 *src = src_;
    for (; c--; ++src) {
        if (*src == ch) return (void *)src;
    }

    return NULL;
}
