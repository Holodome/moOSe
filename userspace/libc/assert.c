#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

__BEGIN_DECLS

void __assertion_failed(const char *msg) {
    printf("ASSERTION FAILED: %s\n", msg);
    abort();
}

__END_DECLS
