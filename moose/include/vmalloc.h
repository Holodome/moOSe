#pragma once

#include <types.h>

int vbrk(void *addr);
void *vsbrk(intptr_t increment);
