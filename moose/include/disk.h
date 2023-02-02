#pragma once

#include <types.h>
#include <kstdio.h>

int disk_init(void);

ssize_t disk_read(void *buf, size_t size);
ssize_t disk_write(const void *buf, size_t size);
ssize_t disk_seek(off_t off, int whence);

ssize_t disk_partition_read(void *buf, size_t size);
ssize_t disk_partition_write(const void *buf, size_t size);
ssize_t disk_partition_seek(off_t off, int whence);
