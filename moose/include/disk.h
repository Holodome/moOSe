#pragma once

#include <kstdio.h>
#include <types.h>

ssize_t disk_read(void *buf, size_t size);
ssize_t disk_write(const void *buf, size_t size);
ssize_t disk_seek(off_t off, int whence);
