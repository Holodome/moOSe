#pragma once

#include <arch/atomic.h>

typedef struct refcount {
    atomic_t refs;
} refcount_t;

void refcount_set(refcount_t *rc, unsigned int value);
unsigned int refcount_read(refcount_t *rc);

void refcount_inc(refcount_t *rc);
void refcount_dec(refcount_t *rc);
int refcount_dec_and_test(refcount_t *rc);

