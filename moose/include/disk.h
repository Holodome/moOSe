#pragma once

#include <kstdio.h>
#include <types.h>

int init_disk(void);

extern struct device *disk_dev;
extern struct device *disk_part_dev;
