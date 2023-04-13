#include <errno.h>
#include <mm/kmalloc.h>
#include <param.h>
#include <string.h>
#include <tty/console.h>

#define PORT_CTL 0x3d4
#define PORT_DAT 0x3d5

struct vga {
    spinlock_t lock;
    u16 *buffer;
};

static void vga_release(struct console *console);
static void vga_clear(struct console *console, size_t x, size_t y,
                      size_t length);
static void vga_write(struct console *console, size_t x, size_t y, int c,
                      enum console_color bg, enum console_color fg);
static void vga_set_cursor(struct console *console, size_t x, size_t y);
static void vga_hide_cursor(struct console *console);
static void vga_show_cursor(struct console *console, size_t x, size_t y);

static const struct console_ops ops = {.release = vga_release,
                                       .clear = vga_clear,
                                       .write = vga_write,
                                       .set_cursor = vga_set_cursor,
                                       .hide_cursor = vga_hide_cursor,
                                       .show_cursor = vga_show_cursor};

int vga_init_console(struct console *console) {
    struct vga *vga = kmalloc(sizeof(*vga));
    if (!vga)
        return -ENOMEM;

    init_spin_lock(&vga->lock);
    // TODO: ioremap
    vga->buffer = FIXUP_PTR((u8 *)0xb8000);
    memset(vga->buffer, 0, 80 * 25 * 2);

    console->private = vga;
    console->ops = &ops;
    console->width = 80;
    console->height = 25;

    return 0;
}

static void vga_release(struct console *console) {
    struct vga *vga = console->private;
    kfree(vga);
}

static void vga_clear(struct console *console, size_t x, size_t y,
                      size_t length) {
    struct vga *vga = console->private;
    cpuflags_t flags = spin_lock_irqsave(&vga->lock);
    size_t idx = y * console->width + x;
    u16 *at = vga->buffer + idx;

    for (; length; --length)
        *at++ = 0x0f20; // space white on black

    spin_unlock_irqrestore(&vga->lock, flags);
}

static void vga_write(struct console *console, size_t x, size_t y, int c,
                      enum console_color bg, enum console_color fg) {
    struct vga *vga = console->private;
    cpuflags_t flags = spin_lock_irqsave(&vga->lock);

    size_t idx = y * console->width + x;

    u16 *at = vga->buffer + idx;
    *at = ((bg & 0x7) << 12) | (fg << 8) | c;

    spin_unlock_irqrestore(&vga->lock, flags);
}

static void vga_set_cursor(struct console *console, size_t x, size_t y) {
    struct vga *vga = console->private;
    cpuflags_t flags = spin_lock_irqsave(&vga->lock);
    u16 cursor = y * console->width + x;

    port_out8(PORT_CTL, 0x0e);
    port_out8(PORT_DAT, cursor >> 8);
    port_out8(PORT_CTL, 0x0f);
    port_out8(PORT_DAT, cursor & 0xff);

    spin_unlock_irqrestore(&vga->lock, flags);
}

static void vga_hide_cursor(struct console *console) {
    struct vga *vga = console->private;
    cpuflags_t flags = spin_lock_irqsave(&vga->lock);
    port_out8(PORT_CTL, 0x0a);
    port_out8(PORT_DAT, 0x20);
    spin_unlock_irqrestore(&vga->lock, flags);
}

static void vga_show_cursor(struct console *console, size_t x, size_t y) {
    vga_set_cursor(console, x, y);
}
