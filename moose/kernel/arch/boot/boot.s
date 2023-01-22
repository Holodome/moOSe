.code16

// Stack pointer value
.equ STACK, 0x8000
// Where to load kernel
.equ KERNEL_OFFSET, 0x1000

boot:
    // Set up registers because bios does not guarantee their contents
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw STACK, %bx
    movb %dl, boot_drive

    cli
    movw %bx, %ss
    movw %ax, %sp
    sti
    cld

    push intro_msg_len
    push $intro_msg
    call print_msg

    // load kernel
    mov KERNEL_OFFSET, %bx
    movb $2, %dh
    movb boot_drive, %dl
    call disk_load

    jmp switch_to_32bit
halt:
    jmp halt

// Prints message with str, len passed on stack
print_msg:
    movw %sp, %bp

    # Get cursor position
    movb $0x03, %ah
    # Set the color for next interrupt and page 0
    movw $0x0007, %bx
    int $0x10
    # Write string, move cursor
    movw $0x1301, %ax
    movw 4(%bp), %cx
    movw 2(%bp), %bp
    int $0x10

    ret

// Reads dh number of sectors from disk to es:bx
disk_load:
    pusha
    push %dx

    movb $0x02, %ah
    movb %dh, %al
    movw $0x0002, %cx
    movb $0x00, %dh

    int $0x13
    jc disk_error

    pop %dx
    cmp %al, %dh
    jne disk_error

    popa
    ret
disk_error:
    push disk_error_msg_len
    push $disk_error_msg
    call print_msg
    jmp halt

// GDT that resembles flat memory model

// null segment
gdt_start: .quad 0x0

// code segment
gdt_code:
    .word 0xffff // segment length
    .word 0x0000 // segment base
    .byte 0x00   // segment base upper bits
    .byte 0x9A   // flags
    .byte 0xCF   // flags + segment length
    .byte 0x00   // segment base upper bits

// data segment
gdt_data:
    .word 0xffff // segment length
    .word 0x0000 // segment base
    .byte 0x00   // segment base upper bits
    .byte 0x9A   // flags
    .byte 0xCF   // flags + segment length
    .byte 0x00   // segment base upper bits

gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1 // size
    .long gdt_start               // address

.equ CODE_SEG, gdt_code - gdt_start
.equ DATA_SEG, gdt_data - gdt_start

.code16
switch_to_32bit:
    cli
    lgdt gdt_descriptor
    movl %cr0, %eax
    orl $0x1, %eax // enable protected mode
    movl %eax, %cr0
    jmp $CODE_SEG, $init_32bit

.code32
init_32bit:
    movw DATA_SEG, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movl 0x90000, %ebp
    movl %esp, %ebp

    jmp KERNEL_OFFSET

boot_drive: .byte 0
intro_msg: .ascii "running moOSe stage 1 bootloader\r\n"
intro_msg_len: .word (. - intro_msg)
disk_error_msg: .ascii "failed to read disk\r\n"
disk_error_msg_len: .word (. - disk_error_msg)
finish_msg: .ascii "bootloader finished\r\n"
finish_msg_len: .word (. - finish_msg)
