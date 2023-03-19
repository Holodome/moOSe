#include <device.h>
#include <drivers/keyboard.h>
#include <drivers/vga.h>

ssize_t tty_read(void *buffer, size_t count) {
    return keyboard_read(buffer, count);
}

ssize_t tty_write(const void *buffer, size_t count) {
    return vga_write(buffer, count);
}

