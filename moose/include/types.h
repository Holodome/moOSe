#pragma once

// FIXME: Adjust architectures
#include "../kernel/arch/types.h"

#define CHECK_TYPE(_type, _size) \
    _Static_assert(sizeof(_type) == _size, "Type " #_type " size does not equal " #_size)

CHECK_TYPE(i8, 1);
CHECK_TYPE(u8, 1);
CHECK_TYPE(i16, 2);
CHECK_TYPE(u16, 2);
CHECK_TYPE(i32, 4);
CHECK_TYPE(u32, 4);

#undef CHECK_TYPE
