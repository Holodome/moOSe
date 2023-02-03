#pragma once

#include <types.h>

void init_memory(void);

void *kmalloc(size_t size);
void *kzalloc(size_t size);
void kfree(void *mem);
