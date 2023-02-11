#pragma once

#include <kstdio.h>
#include <types.h>

int disk_init(void);

extern struct device *disk_dev;
extern struct device *disk_part_dev;
