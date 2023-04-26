/*
 * Atomics based on gcc builtins
 * https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
 * atomic_t is an atomic type with underlying type of int (both signed and
 * unsigned)
 */
#pragma once

typedef struct atomic {
    int v;
} atomic_t;

#define INIT_ATOMIC(_x)                                                        \
    { (_x) }

static inline int atomic_read(const atomic_t *a) {
    return __atomic_load_n(&a->v, __ATOMIC_RELAXED);
}

static inline int atomic_read_acquire(const atomic_t *a) {
    return __atomic_load_n(&a->v, __ATOMIC_ACQUIRE);
}

static inline void atomic_set(atomic_t *a, int v) {
    return __atomic_store_n(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_set_release(atomic_t *a, int v) {
    return __atomic_store_n(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_add(atomic_t *a, int v) {
    __atomic_add_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_add_return(atomic_t *a, int v) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_add_return_relaxed(atomic_t *a, int v) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_add_return_acquire(atomic_t *a, int v) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_add_return_release(atomic_t *a, int v) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_RELEASE);
}

static inline int atomic_fetch_add(atomic_t *a, int v) {
    return __atomic_fetch_add(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_add_relaxed(atomic_t *a, int v) {
    return __atomic_fetch_add(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_add_acquire(atomic_t *a, int v) {
    return __atomic_fetch_add(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_add_release(atomic_t *a, int v) {
    return __atomic_fetch_add(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_sub(atomic_t *a, int v) {
    __atomic_sub_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_sub_return(atomic_t *a, int v) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_sub_return_relaxed(atomic_t *a, int v) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_sub_return_acquire(atomic_t *a, int v) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_sub_return_release(atomic_t *a, int v) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_RELEASE);
}

static inline int atomic_fetch_sub(atomic_t *a, int v) {
    return __atomic_fetch_sub(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_sub_relaxed(atomic_t *a, int v) {
    return __atomic_fetch_sub(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_sub_acquire(atomic_t *a, int v) {
    return __atomic_fetch_sub(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_sub_release(atomic_t *a, int v) {
    return __atomic_fetch_sub(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_and(atomic_t *a, int v) {
    __atomic_and_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_and(atomic_t *a, int v) {
    return __atomic_fetch_and(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_and_relaxed(atomic_t *a, int v) {
    return __atomic_fetch_and(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_and_acquire(atomic_t *a, int v) {
    return __atomic_fetch_and(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_and_release(atomic_t *a, int v) {
    return __atomic_fetch_and(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_or(atomic_t *a, int v) {
    __atomic_or_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_or(atomic_t *a, int v) {
    __atomic_fetch_or(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_or_relaxed(atomic_t *a, int v) {
    return __atomic_fetch_or(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_or_acquire(atomic_t *a, int v) {
    return __atomic_fetch_or(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_or_release(atomic_t *a, int v) {
    return __atomic_fetch_or(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_xor(atomic_t *a, int v) {
    __atomic_xor_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_xor(atomic_t *a, int v) {
    return __atomic_fetch_xor(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_xor_relaxed(atomic_t *a, int v) {
    return __atomic_fetch_xor(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_xor_acquire(atomic_t *a, int v) {
    return __atomic_fetch_xor(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_xor_release(atomic_t *a, int v) {
    return __atomic_fetch_xor(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_nand(atomic_t *a, int v) {
    __atomic_nand_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_nand(atomic_t *a, int v) {
    return __atomic_fetch_nand(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_nand_relaxed(atomic_t *a, int v) {
    return __atomic_fetch_nand(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_nand_acquire(atomic_t *a, int v) {
    return __atomic_fetch_nand(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_nand_release(atomic_t *a, int v) {
    return __atomic_fetch_nand(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_inc(atomic_t *a) {
    __atomic_add_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_inc_return(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_SEQ_CST);
}

static inline int atomic_inc_return_relaxed(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_inc_return_acquire(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline int atomic_inc_return_release(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_RELEASE);
}

static inline int atomic_fetch_inc(atomic_t *a) {
    return __atomic_fetch_add(&a->v, 1, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_inc_relaxed(atomic_t *a) {
    return __atomic_fetch_add(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_inc_acquire(atomic_t *a) {
    return __atomic_fetch_add(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_inc_release(atomic_t *a) {
    return __atomic_fetch_add(&a->v, 1, __ATOMIC_RELEASE);
}

static inline void atomic_dec(atomic_t *a) {
    __atomic_sub_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_dec_return(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_SEQ_CST);
}

static inline int atomic_dec_return_relaxed(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_dec_return_acquire(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline int atomic_dec_return_release(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_RELEASE);
}

static inline int atomic_fetch_dec(atomic_t *a) {
    return __atomic_fetch_sub(&a->v, 1, __ATOMIC_SEQ_CST);
}

static inline int atomic_fetch_dec_relaxed(atomic_t *a) {
    return __atomic_fetch_sub(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_fetch_dec_acquire(atomic_t *a) {
    return __atomic_fetch_sub(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline int atomic_fetch_dec_release(atomic_t *a) {
    return __atomic_fetch_sub(&a->v, 1, __ATOMIC_RELEASE);
}

static inline int atomic_xchg(atomic_t *a, int v) {
    return __atomic_exchange_n(&a->v, v, __ATOMIC_SEQ_CST);
}

static inline int atomic_xchg_relaxed(atomic_t *a, int v) {
    return __atomic_exchange_n(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_xchg_acquire(atomic_t *a, int v) {
    return __atomic_exchange_n(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_xchg_release(atomic_t *a, int v) {
    return __atomic_exchange_n(&a->v, v, __ATOMIC_RELEASE);
}

static inline int atomic_try_cmpxchg(atomic_t *a, int *old, int new) {
    return __atomic_compare_exchange_n(&a->v, old, new, 0, __ATOMIC_SEQ_CST,
                                       __ATOMIC_RELAXED);
}

static inline int atomic_try_cmpxchg_relaxed(atomic_t *a, int *old, int new) {
    return __atomic_compare_exchange_n(&a->v, old, new, 0, __ATOMIC_RELAXED,
                                       __ATOMIC_RELAXED);
}

static inline int atomic_try_cmpxchg_acquire(atomic_t *a, int *old, int new) {
    return __atomic_compare_exchange_n(&a->v, old, new, 0, __ATOMIC_ACQUIRE,
                                       __ATOMIC_RELAXED);
}

static inline int atomic_try_cmpxchg_release(atomic_t *a, int *old, int new) {
    return __atomic_compare_exchange_n(&a->v, old, new, 0, __ATOMIC_RELEASE,
                                       __ATOMIC_RELAXED);
}

static inline int atomic_cmpxchg(atomic_t *a, int old, int new) {
    atomic_try_cmpxchg(a, &old, new);
    return old;
}

static inline int atomic_cmpxchg_relaxed(atomic_t *a, int old, int new) {
    atomic_try_cmpxchg_relaxed(a, &old, new);
    return old;
}

static inline int atomic_cmpxchg_acquire(atomic_t *a, int old, int new) {
    atomic_try_cmpxchg_acquire(a, &old, new);
    return old;
}

static inline int atomic_cmpxchg_release(atomic_t *a, int old, int new) {
    atomic_try_cmpxchg_release(a, &old, new);
    return old;
}
