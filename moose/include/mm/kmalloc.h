#pragma once

#include <types.h>

void init_kmalloc(void);

void *kmalloc(size_t size);
void *kzalloc(size_t size);
void kfree(void *mem);

char *kstrdup(const char *src);
