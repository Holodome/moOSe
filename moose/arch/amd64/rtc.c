#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <arch/amd64/rtc.h>

#define RATE 8
#define FREQUENCY (32768 >> (RATE - 1))

static volatile u64 jiffies;

static void timer_interrupt(const struct registers_state *regs
                            __attribute__((unused))) {
    ++jiffies;
    (void)cmos_read(0x8c);
}

void init_rtc(void) {
    register_isr(8, timer_interrupt);

    u8 prev;
    u8 rate = RATE;

    // enable interrupt
    cli();
    prev = cmos_read(0x8b);
    cmos_write(0x8b, prev | 0x40);
    // register frequency
    prev = cmos_read(0x8a);
    cmos_write(0x8a, (prev & 0xf0) | rate);
    sti();
}

u32 get_jiffies(void) { return (u32)jiffies; }
u64 get_jiffies64(void) { return jiffies; }
u64 jiffies64_to_msecs(u64 jiffies) { return jiffies / FREQUENCY; }
u64 msecs_to_jiffies64(u64 msecs) { return msecs * FREQUENCY; }
u32 jiffies_to_msecs(u32 jiffies) { return jiffies / FREQUENCY; }
u32 msecs_to_jiffies(u32 msecs) { return msecs * FREQUENCY; }
