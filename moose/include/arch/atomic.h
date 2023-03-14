/*
 * Atomics based on gcc builtins
 * https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
 * raw_atomic family of functions are simply wrappers over gcc intrinsics
 * atomic_t is an atomic type of relaxed ordering and size of int
 * atomic64_t is an 64-bit atomic_t
 * NOTE that we use linux-like API but we are not strictly linux-conformant
 * linux memory ordering on atomics with return value requires two smb_mb
 * barriers, which we do not provide. They seem have no effect on x86,
 * however. To not bring confusion, we do not provide 'relaxed' function
 * variants.
 */
#pragma once

typedef struct atomic {
    int v;
} atomic_t;

static inline int atomic_read(const atomic_t *a) {
    return __atomic_load_n(&a->v, __ATOMIC_RELAXED);
}

static inline int atomic_read_acquire(const atomic_t *a) {
    return __atomic_load_n(&a->v, __ATOMIC_ACQUIRE);
}

static inline void atomic_set(int v, atomic_t *a) {
    return __atomic_store_n(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_set_release(int v, atomic_t *a) {
    return __atomic_store_n(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_add(int v, atomic_t *a) {
    __atomic_add_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_add_return(int v, atomic_t *a) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_add_return_acquire(int v, atomic_t *a) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_add_return_release(int v, atomic_t *a) {
    return __atomic_add_fetch(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_fetch_add(int v, atomic_t *a) {
    __atomic_fetch_add(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_add_acquire(int v, atomic_t *a) {
    __atomic_fetch_add(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_add_release(int v, atomic_t *a) {
    __atomic_fetch_add(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_sub(int v, atomic_t *a) {
    __atomic_sub_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_sub_return(int v, atomic_t *a) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline int atomic_sub_return_acquire(int v, atomic_t *a) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline int atomic_sub_return_release(int v, atomic_t *a) {
    return __atomic_sub_fetch(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_fetch_sub(int v, atomic_t *a) {
    __atomic_fetch_sub(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_sub_acquire(int v, atomic_t *a) {
    __atomic_fetch_sub(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_sub_release(int v, atomic_t *a) {
    __atomic_fetch_sub(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_and(int v, atomic_t *a) {
    __atomic_and_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_and(int v, atomic_t *a) {
    __atomic_fetch_and(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_and_acquire(int v, atomic_t *a) {
    __atomic_fetch_and(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_and_release(int v, atomic_t *a) {
    __atomic_fetch_and(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_or(int v, atomic_t *a) {
    __atomic_or_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_or(int v, atomic_t *a) {
    __atomic_fetch_or(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_or_acquire(int v, atomic_t *a) {
    __atomic_fetch_or(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_or_release(int v, atomic_t *a) {
    __atomic_fetch_or(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_xor(int v, atomic_t *a) {
    __atomic_xor_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_xor(int v, atomic_t *a) {
    __atomic_fetch_xor(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_xor_acquire(int v, atomic_t *a) {
    __atomic_fetch_xor(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_xor_release(int v, atomic_t *a) {
    __atomic_fetch_xor(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_andnot(int v, atomic_t *a) {
    __atomic_nand_fetch(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_andnot(int v, atomic_t *a) {
    __atomic_fetch_nand(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_andnot_acquire(int v, atomic_t *a) {
    __atomic_fetch_nand(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_andnot_release(int v, atomic_t *a) {
    __atomic_fetch_nand(&a->v, v, __ATOMIC_RELEASE);
}

static inline void atomic_inc(atomic_t *a) {
    __atomic_add_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_inc_return(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_inc_return_acquire(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline int atomic_inc_return_release(atomic_t *a) {
    return __atomic_add_fetch(&a->v, 1, __ATOMIC_RELEASE);
}

static inline void atomic_fetch_inc(atomic_t *a) {
    __atomic_fetch_add(&a->v, 1, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_inc_acquire(atomic_t *a) {
    __atomic_fetch_add(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_inc_release(atomic_t *a) {
    __atomic_fetch_add(&a->v, 1, __ATOMIC_RELEASE);
}

static inline void atomic_dec(atomic_t *a) {
    __atomic_sub_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_dec_return(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_RELAXED);
}

static inline int atomic_dec_return_acquire(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline int atomic_dec_return_release(atomic_t *a) {
    return __atomic_sub_fetch(&a->v, 1, __ATOMIC_RELEASE);
}

static inline void atomic_fetch_dec(atomic_t *a) {
    __atomic_fetch_sub(&a->v, 1, __ATOMIC_RELAXED);
}

static inline void atomic_fetch_dec_acquire(atomic_t *a) {
    __atomic_fetch_sub(&a->v, 1, __ATOMIC_ACQUIRE);
}

static inline void atomic_fetch_dec_release(atomic_t *a) {
    __atomic_fetch_sub(&a->v, 1, __ATOMIC_RELEASE);
}

static inline void atomic_xchg(atomic_t *a, int v) {
    __atomic_exchange_n(&a->v, v, __ATOMIC_RELAXED);
}

static inline void atomic_xchg_acquire(atomic_t *a, int v) {
    __atomic_exchange_n(&a->v, v, __ATOMIC_ACQUIRE);
}

static inline void atomic_xchg_release(atomic_t *a, int v) {
    __atomic_exchange_n(&a->v, v, __ATOMIC_RELEASE);
}

static inline int atomic_try_cmpxchg(atomic_t *a, int old, int new) {
    return __atomic_compare_exchange_n(&a->v, &old, new, 0, __ATOMIC_RELAXED,
                                       __ATOMIC_RELAXED);
}

static inline int atomic_try_cmpxchg_acquire(atomic_t *a, int old, int new) {
    return __atomic_compare_exchange_n(&a->v, &old, new, 0, __ATOMIC_RELAXED,
                                       __ATOMIC_ACQUIRE);
}

static inline int atomic_try_cmpxchg_release(atomic_t *a, int old, int new) {
    return __atomic_compare_exchange_n(&a->v, &old, new, 0, __ATOMIC_RELAXED,
                                       __ATOMIC_RELEASE);
}

static inline int atomic_add_unless(atomic_t *a, int s, int u) {
    int c = atomic_read(a);
    do {
        if (__builtin_expect(c == u, 0)) return 0;
    } while (!atomic_try_cmpxchg(a, c, s + c));
    return 1;
}

static inline int atomic_inc_not_zero(atomic_t *a) {
    return atomic_add_unless(a, 1, 0);
}
