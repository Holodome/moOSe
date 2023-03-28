#pragma once

#include <types.h>

ssize_t tty_read(void *buffer, size_t count);
ssize_t tty_write(const void *buffer, size_t count);
