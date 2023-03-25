#include <drivers/keyboard.h>
#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <arch/cpu.h>
#include <drivers/vga.h>
#include <kstdio.h>
#include <sched/spinlock.h>

#define PORT 0x60
#define BACKSPACE 0x0e
#define ENTER 0x1c
#define SC_MAX 57

static struct {
    spinlock_t read_lock;
    spinlock_t lock;
    atomic_t is_listening;
    u8 *dst_start;
    u8 *dst;
    u8 *dst_end;
} keyboard = {.read_lock = SPIN_LOCK_INIT(), .lock = SPIN_LOCK_INIT()};

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-', '=',  '?',
    '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',  '[', ']', '?',  '?',
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '?', '\\', 'Z',
    'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', '?', '?',  '?', ' '};

static void handle_input(u8 codepoint) {
    if (codepoint > SC_MAX) return;

    if (codepoint == BACKSPACE) {
        if (keyboard.dst > keyboard.dst_start) {
            --keyboard.dst;
            vga_backspace();
        }
    } else if (codepoint == ENTER) {
        kputc('\n');
        atomic_set(&keyboard.is_listening, 0);
    } else {
        if (keyboard.dst < keyboard.dst_end) {
            *keyboard.dst = sc_ascii[codepoint];
            kputc(*keyboard.dst++);
        }
    }
}

static void keyboard_isr(struct registers_state *regs __attribute__((unused))) {
    // read value so interrupt is flushed
    u8 codepoint = port_in8(PORT);

    if (atomic_read(&keyboard.is_listening)) {
        u64 flags;
        if (spin_trylock_irqsave(&keyboard.lock, flags)) {
            handle_input(codepoint);
            spin_unlock_irqrestore(&keyboard.lock, flags);
        }
    }
}

void init_keyboard(void) { register_isr(1, keyboard_isr); }

ssize_t keyboard_read(void *buffer, size_t count) {
    spin_lock(&keyboard.read_lock);
    u64 flags;
    spin_lock_irqsave(&keyboard.lock, flags);
    atomic_set(&keyboard.is_listening, 1);
    keyboard.dst = keyboard.dst_start = buffer;
    keyboard.dst_end = keyboard.dst + count;
    spin_unlock_irqrestore(&keyboard.lock, flags);

    while (atomic_read(&keyboard.is_listening)) spinloop_hint();

    spin_lock_irqsave(&keyboard.lock, flags);
    size_t result = keyboard.dst - keyboard.dst_start;
    spin_unlock_irqrestore(&keyboard.lock, flags);
    spin_unlock(&keyboard.read_lock);

    return result;
}

