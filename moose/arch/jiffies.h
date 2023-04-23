#pragma once

#include <moose/types.h>

u64 get_jiffies(void);
u32 msecs_to_jiffies(u64 msecs);
u64 jiffies_to_msecs(u64 jiffies);
