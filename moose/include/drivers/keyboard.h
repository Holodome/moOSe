#pragma once

#include <types.h>

void init_keyboard(void);

ssize_t keyboard_read(void *buffer, size_t count);
