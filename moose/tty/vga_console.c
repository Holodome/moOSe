#include <errno.h>
#include <mm/kmalloc.h>
#include <param.h>
#include <tty/console.h>

struct vga {
    spinlock_t lock;
    u16 *buffer;
};

static void vga_release(struct console *console);
static void vga_clear(size_t x, size_t y, size_t length);
static void vga_write_ex(size_t x, size_t y, int c, enum console_color bg,
                         enum console_color fg);
static void vga_write(size_t x, size_t y, int c);
static void vga_flush(size_t x __unused, size_t y __unused,
                      size_t width __unused, size_t height __unused) {
}
static void vga_set_cursor(size_t x, size_t y);

const static struct console_ops ops = {.release = vga_release,
                                       .clear = vga_clear,
                                       .write_ex = vga_write_ex,
                                       .write = vga_write,
                                       .flush = vga_flush,
                                       .set_cursor = vga_set_cursor};

int vga_init_console(struct console *console) {
    struct vga *vga = kmalloc(sizeof(*vga));
    if (!vga)
        return -ENOMEM;

    spin_lock_init(&vga->lock);
    // TODO: ioremap
    vga->buffer = FIXUP_PTR((u8 *)0xb8000);

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
