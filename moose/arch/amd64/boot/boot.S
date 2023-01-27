.code16

boot:
    mov $0x9000, %bp
    mov %bp, %sp

    xor %ax, %ax
    mov %ax, %ss
    mov %ax, %ds

    mov $0x500, %si
    movb $0x10, (%si)
    movb $0, 1(%si)
    movw $2, 2(%si)
    movw $0x1000, 4(%si)
    movw $0, 6(%si)
    movw $1, 8(%si)
    movw $0, 10(%si)
    movw $0, 12(%si)
    movw $0, 14(%si)
    mov $0x42, %ah
    int $0x13
    jc halt

    jmp switch_to_32bit
halt:
    mov $'E', %al
    mov $0x0e, %ah
    mov $0x00, %bh
    mov $0x07, %bl
    int $0x10
    jmp halt

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

    mov $0x90000, %ebp
    mov %ebp, %esp

    ljmp $0x0008,$0x1000

boot_drive: .byte 0
