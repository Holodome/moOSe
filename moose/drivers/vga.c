#include <arch/amd64/asm.h>
#include <assert.h>
#include <param.h>
#include <sched/locks.h>

#define WIDTH 80
#define HEIGHT 25

#define TEXTBUF FIXUP_PTR((volatile u8 *)0xb8000)
#define WHITE_ON_BLACK 0x0f

#define PORT_CTL 0x3d4
#define PORT_DAT 0x3d5

#define CTL_CURSOR_HI 14
#define CTL_CURSOR_LO 15

static spinlock_t lock = SPIN_LOCK_INIT();

u32 console_get_width(void) {
    return WIDTH;
}
u32 console_get_height(void) {
    return HEIGHT;
}

static u16 get_cursor(void) {
    port_out8(PORT_CTL, CTL_CURSOR_HI);
    u16 offset = port_in8(PORT_DAT) << 8;
    port_out8(PORT_CTL, CTL_CURSOR_LO);
    offset |= port_in8(PORT_DAT);
    return offset;
}

static void set_cursor(u16 cursor) {
    port_out8(PORT_CTL, CTL_CURSOR_HI);
    port_out8(PORT_DAT, cursor >> 8);
    port_out8(PORT_CTL, CTL_CURSOR_LO);
    port_out8(PORT_DAT, cursor & 0xff);
}

static void set_char(u16 offset, int symb) {
    expects(offset < (WIDTH * HEIGHT));
    volatile u8 *slot = TEXTBUF + offset * 2;
    *slot++ = symb;
    *slot = WHITE_ON_BLACK;
}

static u16 scroll_line(void) {
    // memmove text buffer
    volatile u16 *dst = (volatile u16 *)TEXTBUF;
    const volatile u16 *src = (volatile u16 *)TEXTBUF + WIDTH;
    u16 count = WIDTH * (HEIGHT - 1);
    u16 last_line_offset = count;
    while (count--)
        *dst++ = *src++;

    for (u16 i = 0; i < WIDTH; ++i)
        set_char(last_line_offset + i, ' ');

    return last_line_offset;
}

ssize_t vga_write(const void *buf_, size_t buf_size) {
    cpuflags_t flags;
    spin_lock_irqsave(&lock, flags);
    u16 cursor = get_cursor();
    if (cursor > WIDTH * HEIGHT)
        cursor = 0;
    const char *buf = buf_;
    for (size_t i = 0; i < buf_size; ++i) {
        int c = buf[i];

        if (cursor >= WIDTH * HEIGHT)
            cursor = scroll_line();

        if (c == '\n') {
            cursor = ((cursor / WIDTH) + 1) * WIDTH;
        } else {
            set_char(cursor, c);
            ++cursor;
        }
    }

    if (cursor >= WIDTH * HEIGHT)
        cursor = scroll_line();

    set_cursor(cursor);
    spin_unlock_irqrestore(&lock, flags);
    return buf_size;
}

void vga_clear(void) {
    cpuflags_t flags;
    spin_lock_irqsave(&lock, flags);
    for (u16 i = 0; i < WIDTH * HEIGHT; ++i)
        set_char(i, ' ');

    set_cursor(0);
    spin_unlock_irqrestore(&lock, flags);
}

void vga_backspace(void) {
    cpuflags_t flags;
    spin_lock_irqsave(&lock, flags);
    u16 cursor = get_cursor() - 1;
    set_char(cursor, ' ');
    set_cursor(cursor);
    spin_unlock_irqrestore(&lock, flags);
}
