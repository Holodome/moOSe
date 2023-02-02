#include <arch/processor.h>

void halt(void) {
    asm volatile("hlt");
    __builtin_unreachable();
}

void disable_interrupts(void) { asm volatile("cli"); }

void enable_interrupts(void) { asm volatile ("sti"); }

void pause(void) { asm volatile("pause"); }
