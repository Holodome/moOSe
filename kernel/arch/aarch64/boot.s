// put this code at the start of kernel image
.section ".text.boot"

// this is execution start point
.global _start

_start:
    // check processor ID is zero (which means we are at the main core)
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, main_core
    // if we are not on the main core, hang
secondary_core:
    b secondary_core
main_core:
    // set stack pointer
    ldr x1, =_start
    mov sp, x1

    // zero BSS
    ldr x1, =__bss_start
    ldr w2, =__bss_size
bss_loop:
    cbz w2, execute_main
    str xzr, [x1], #8
    sub w2, w2, #1
    cbnz w2, bss_loop

execute_main:
    bl main
    // if main returns, halt
    b secondary_core
