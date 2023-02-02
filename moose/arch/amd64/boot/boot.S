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
    movw $40, 2(%si)
    movw $0x1000, 4(%si)
    movw $0, 6(%si)
    movw $1, 8(%si)
    movw $0, 10(%si)
    movw $0, 12(%si)
    movw $0, 14(%si)
    mov $0x42, %ah
    int $0x13
    jc halt

    mov $0x1000, %eax
    jmp %eax
halt:
    jmp halt

boot_drive: .byte 0
