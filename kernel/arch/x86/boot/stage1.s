.code16
.section ".boot1.text"

# Stack pointer value
.equ STACK, 0x8000
# Where to load kernel
.equ KERNEL_OFFSET, 0x1000

boot:
    # Set up registers because bios does not guarantee their contents
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

    mov KERNEL_OFFSET, %bx
    movb $2, %dh
    movb boot_drive, %dl
    call disk_load

    push finish_msg_len
    push $finish_msg
    call print_msg

    jmp .

    addw $8, %sp

    jmp KERNEL_OFFSET
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

# Reads dh number of sectors from disk to es:bx
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

boot_drive: .byte 0
intro_msg: .ascii "running moOSe stage 1 bootloader\r\n"
intro_msg_len: .word (. - intro_msg)
disk_error_msg: .ascii "failed to read disk\r\n"
disk_error_msg_len: .word (. - disk_error_msg)
finish_msg: .ascii "jumping to stage 2 bootloader\r\n"
finish_msg_len: .word (. - finish_msg)
