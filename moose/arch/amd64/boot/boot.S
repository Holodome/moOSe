.code16

boot:
    movw $0x9000, %bp
    movw %bp, %sp

    xorw %ax, %ax
    movw %ax, %ss
    movw %ax, %ds

    /* size of stage 2 bootloader is located at 0x01b0 */
    movw $0x7c00 + 0x01b0, %di
    movw (%di), %ax
    movw %ax, 2(%si)
    movw $0x1000, 4(%si)
    movw $0, 6(%si)
    movw $1, 8(%si)
    movw $0, 10(%si)
    movw $0, 12(%si)
    movw $0, 14(%si)
    movb $0x42, %ah
    int $0x13
    jc halt

    movl $0x1000, %eax
    jmp *%eax
halt:
    jmp halt
