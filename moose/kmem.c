#include "kmem.h"

u32 strlen(const char *str) {
    const char *cur = str;
    while (*++cur);
    return cur - str;
}
