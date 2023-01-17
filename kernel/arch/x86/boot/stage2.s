.code16 
.section ".boot2.text"

start:
    push intro_msg_len
    push $intro_msg
    call print_msg

    jmp .

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

intro_msg: .ascii "running moOSe stage 2 bootloader\r\n"
intro_msg_len: .word (. - intro_msg)
