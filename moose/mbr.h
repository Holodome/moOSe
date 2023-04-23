#pragma once

#include <moose/types.h>

#define MBR_PARTITION_OFFSET 0x01be
#define MBR_PARTITION_SIZE 16

// status value
#define MBR_PARTITION_BOOTABLE 0x80

struct mbr_partition {
    u8 status;
    u8 first_chs[3];
    u8 type;
    u8 last_chs[3];
    u32 addr;
    u32 size;
};

static_assert(sizeof(struct mbr_partition) == MBR_PARTITION_SIZE);
