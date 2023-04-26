#include <moose/arch/refcount.h>

void refcount_set(refcount_t *rc, unsigned int value) {
    atomic_set(&rc->refs, value);
}

unsigned int refcount_read(refcount_t *rc) {
    return atomic_read(&rc->refs);
}
void refcount_inc(refcount_t *rc) {
    atomic_inc(&rc->refs);
}
void refcount_dec(refcount_t *rc) {
    atomic_fetch_dec_release(&rc->refs);
}
int refcount_dec_and_test(refcount_t *rc) {
    return atomic_dec_return_release(&rc->refs) == 0;
}
