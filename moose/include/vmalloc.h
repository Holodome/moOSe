#pragma once

#include <types.h>

int vbrk(void *addr);
void *vsbrk(intptr_t increment);

void *vmalloc(size_t size);
void vfree(const void *addr);
