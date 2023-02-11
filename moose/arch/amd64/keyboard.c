#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <arch/amd64/keyboard.h>
#include <arch/amd64/vga.h>
#include <kstdio.h>
#include <tty.h>

#define PORT 0x60
#define BACKSPACE 0x0e
#define ENTER 0x1c
#define SC_MAX 57

static volatile int is_listening;
static volatile u8 *volatile dst_start;
static volatile u8 *volatile dst;
static volatile u8 *volatile dst_end;

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  '?',
    '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',  '[', ']', '?',  '?',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z',
    'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '?', '?',  '?', ' '};

static void keyboard_isr(const struct registers_state *regs
                         __attribute__((unused))) {
    if (!is_listening) {
        return;
    }

    u8 codepoint = port_in8(PORT);
    if (codepoint > SC_MAX)
        return;

    if (codepoint == BACKSPACE) {
        if (dst > dst_start) {
            --dst;
            vga_backspace();
        }
    } else if (codepoint == ENTER) {
        kputc('\n');
        is_listening = 0;
    } else {
        if (dst < dst_end) {
            *dst = sc_ascii[codepoint];
            kputc(*dst++);
        }
    }
}

void init_keyboard(void) { register_isr(1, keyboard_isr); }

ssize_t keyboard_read(void *buffer, size_t count) {
    is_listening = 1;
    dst = dst_start = buffer;
    dst_end = dst + count;

    while (is_listening)
        ;

    return dst - dst_start;
}
