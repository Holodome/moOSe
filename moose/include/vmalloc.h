#pragma once

#include <types.h>

#define VMALLOC_BASE ((void *)0xffffc90000000000)
#define VMALLOC_LIMIT ((void *)0xffffe8ffffffffff)

int vbrk(void *addr);
void *vsbrk(ssize_t increment);

void *vmalloc(size_t size);
void vfree(const void *addr);
