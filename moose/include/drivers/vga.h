#pragma once

#include <types.h>

ssize_t vga_write(const void *buf_, size_t buf_size);
void vga_clear(void);
void vga_backspace(void);
