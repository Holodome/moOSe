#pragma once

#include <kstdio.h>
#include <types.h>

void init_disk(void);

extern struct blk_device *disk_dev;
extern struct blk_device *disk_part_dev;
extern struct blk_device *disk_part1_dev;
