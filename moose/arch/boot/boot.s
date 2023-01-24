.code16

boot:
    mov $0x9000, %bp
    mov %bp, %sp

    mov $0x1000, %bx
    mov $0x31, %dh
    mov $0x00, %dl

    pusha
    push %dx
    mov $0x02, %ah
    mov %dh, %al      // sectors to read
    mov $0x02, %cl      // sector number
    mov $0x00, %ch      // cylinder
    mov $0x00, %dh      // head

    int $0x13
    jc halt

    jmp switch_to_32bit
halt:
    jmp halt

// Prints message with str, len passed on stack
print_msg:
    pusha

    # Get cursor position
    mov $0x03, %ah
    # Set the color for next interrupt and page 0
    mov $0x0007, %bx
    int $0x10
    # Write string, move cursor
    mov $0x1301, %ax
    mov disk_error_msg_len, %cx
    mov $disk_error_msg, %bp
    int $0x10

    popa
    ret

disk_error:
    //push disk_error_msg_len
    //push $disk_error_msg
    //call print_msg
    jmp halt

// GDT that resembles flat memory model

// null segment
gdt_start:
    .quad 0x0

// code segment
gdt_code:
    .word 0xffff // segment length
    .word 0x0000 // segment base
    .byte 0x00   // segment base upper bits
    .byte 0b10011010   // flags
    .byte 0b11001111   // flags + segment length
    .byte 0x00   // segment base upper bits

// data segment
gdt_data:
    .word 0xffff // segment length
    .word 0x0000 // segment base
    .byte 0x00   // segment base upper bits
    .byte 0b10010010   // flags
    .byte 0b11001111   // flags + segment length
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
    ljmp $0x0008, $init_32bit

.code32
init_32bit:
    mov $0x0010, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    movl $0x90000, %ebp
    movl %ebp, %esp

    ljmp $0x0008,$0x1000

boot_drive: .byte 0
intro_msg: .ascii "running moOSe stage 1 bootloader\r\n"
intro_msg_len: .word (. - intro_msg)
disk_error_msg: .ascii "failed to read disk\r\n"
disk_error_msg_len: .word (. - disk_error_msg)
finish_msg: .ascii "bootloader finished\r\n"
finish_msg_len: .word (. - finish_msg)
