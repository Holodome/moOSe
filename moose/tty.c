#include <arch/amd64/keyboard.h>
#include <arch/amd64/vga.h>
#include <device.h>

static ssize_t tty_read(struct device *device __attribute__((unused)),
                        void *buffer, size_t count) {
    return keyboard_read(buffer, count);
}

static ssize_t tty_write(struct device *device __attribute__((unused)),
                         const void *buffer, size_t count) {
    return vga_write(buffer, count);
}

static struct device tty_device_ = {
    .name = "tty", .ops = {.read = tty_read, .write = tty_write}};
struct device *tty_device = &tty_device_;

