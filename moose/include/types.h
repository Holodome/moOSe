#pragma once

<<<<<<< HEAD
#include <arch/amd64/types.h>
=======
#define static_assert(_x) _Static_assert(_x, #_x)
#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(*_arr))

#include "../arch/amd64/types.h"
>>>>>>> refs/remotes/origin/dev
