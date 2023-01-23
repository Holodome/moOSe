#pragma once

typedef char *va_list;

#define va_start(_list, _arg)                                                  \
    do {                                                                       \
        _list = (char *)&_arg + sizeof(_arg);                                  \
    } while (0)

#define va_arg(_list, _type)                                                   \
    ({                                                                         \
        _type val = *(_type *)_list;                                           \
        _list += sizeof(_type);                                                \
        val;                                                                   \
    })

#define va_end(_list)                                                          \
    do {                                                                       \
        _list = NULL;                                                          \
    } while (0)
