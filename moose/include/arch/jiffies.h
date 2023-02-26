#pragma once

#include <types.h>

u32 get_jiffies(void);
u64 get_jiffies64(void);

u32 jiffies_to_msecs(u32 jiffies);
u32 msecs_to_jiffies(u32 msecs);

u64 jiffies64_to_msecs(u64 jiffies);
u64 msecs_to_jiffies64(u64 msecs);

