#include <arch/amd64/idt.h>
#include <arch/amd64/io.h>
#include <arch/amd64/rtc.h>
#include <arch/processor.h>

#define RATE 8
#define FREQUENCY (32768 >> (RATE - 1))

static size_t ticks;

size_t get_systemtick(void) { return ticks; }
size_t get_seconds(void) { return ticks / FREQUENCY; }

static void isr(const struct registers_state *regs __attribute__((unused))) {
    ++ticks;
    (void)cmos_read(0x8c);
}

void init_rtc(void) {
    register_isr(8, isr);

    u8 prev;
    u8 rate = RATE;

    // enable interrupt
    disable_interrupts();
    prev = cmos_read(0x8b);
    cmos_write(0x8b, prev | 0x40);
    enable_interrupts();
    // register frequency
    disable_interrupts();
    prev = cmos_read(0x8a);
    cmos_write(0x8a, (prev & 0xf0) | rate);
    enable_interrupts();
}
