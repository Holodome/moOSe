#include <errno.h>
#include <kstdio.h>

int errno;

char *strerror(int errnum) {
    static char buf[64];
    static const char *strs[] = {
#define E(_name, _str) _str,
        ERRLIST
#undef E
    };

    const char *str = NULL;
    if (errnum == 0) {
        str = "No error information";
    } else if (errnum - 1 < (int)ARRAY_SIZE(strs)) {
        str = strs[errnum - 1];
    }

    snprintf(buf, sizeof(buf), "%s", str);
    return buf;
}
