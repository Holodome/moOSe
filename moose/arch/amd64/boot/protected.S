#include <moose/param.h>

.global switch_to_32bit
/* GDT that resembles flat memory model */

gdt_start:
    .quad 0x0

gdt_code:
    .word 0xffff /* segment length */
    .word 0x0000 /* segment base */
    .byte 0x00   /* segment base upper bits */
    .byte 0b10011010   /* flags */
    .byte 0b11001111   /* flags + segment length */
    .byte 0x00   /* segment base upper bits */

gdt_data:
    .word 0xffff /* segment length */
    .word 0x0000 /* segment base */ 
    .byte 0x00   /* segment base upper bits */
    .byte 0b10010010   /* flags */
    .byte 0b11001111   /* flags + segment length */
    .byte 0x00   /* segment base upper bits */

gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1 /* size */
    .long gdt_start               /* address */

.code16
switch_to_32bit:
    cli
    lgdt gdt_descriptor
    mov %cr0, %eax
    or $0x1, %eax /* enable protected mode */
    mov %eax, %cr0
    ljmp $0x0008, $init_32bit

.code32
init_32bit:
    mov $0x0010, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov $KERNEL_INITIAL_STACK, %ebp
    mov %ebp, %esp

    ljmp $0x0008, $start32
