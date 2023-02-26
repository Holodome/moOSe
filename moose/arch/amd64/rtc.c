#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <arch/amd64/rtc.h>
#include <arch/cpu.h>
#include <assert.h>
#include <kmem.h>
#include <kstdio.h>
#include <kthread.h>

#define RATE 8
#define FREQUENCY (32768 >> (RATE - 1))

static volatile u64 jiffies;

static void timer_interrupt(struct registers_state *regs) {
    ++jiffies;
    (void)cmos_read(0x8c);
    memcpy((void *)&current->regs, regs, sizeof(*regs));

    struct task *next_task =
        list_next_or_null(&current->list, &tasks, struct task, list);
    if (!next_task) {
        assert(tasks.next != &tasks);
        next_task = list_entry(tasks.next, struct task, list);
    }

    if (current != next_task) {
        current = next_task;
        memcpy(regs, (void *)&current->regs, sizeof(*regs));
    }
}

void init_rtc(void) {
    register_isr(8, timer_interrupt);

    u8 prev;
    u8 rate = RATE;

    irq_disable();
    prev = cmos_read(0x8b);
    cmos_write(0x8b, prev | 0x40);
    // register frequency
    prev = cmos_read(0x8a);
    cmos_write(0x8a, (prev & 0xf0) | rate);
    irq_enable();
}

u32 get_jiffies(void) { return (u32)jiffies; }
u64 get_jiffies64(void) { return jiffies; }
u64 jiffies64_to_msecs(u64 jiffies) { return jiffies * 1000 / FREQUENCY; }
u64 msecs_to_jiffies64(u64 msecs) { return msecs * 1000 * FREQUENCY; }
u32 jiffies_to_msecs(u32 jiffies) { return jiffies * 1000 / FREQUENCY; }
u32 msecs_to_jiffies(u32 msecs) { return msecs * 1000 * FREQUENCY; }
