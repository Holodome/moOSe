#pragma once

#include "types.h"

u32 console_get_width(void);
u32 console_get_height(void);
void console_get_cursor(u32 *x, u32 *y);

void console_set_cursor(u32 x, u32 y);
void console_print(const char *buf, size_t buf_size);
void console_clear(void);
