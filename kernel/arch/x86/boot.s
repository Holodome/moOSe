.code16
.global _start
.text

_start:
    # Set up registers because bios does not guarantee their contents
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw $0x8000, %bx

    cli
    movw %bx, %ss
    movw %ax, %sp
    sti
    cld

    push intro_msg_len
    push $intro_msg
    call print_msg
halt:
    jmp halt

# Prints message with str, len passed on stack
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

intro_msg: .ascii "running moOSe bootloader"
intro_msg_len: .word (. - intro_msg)
