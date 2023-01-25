//
// This file contains code for 'print' procedure 
// that outputs null-terminated string using VGA MMIO
//
.global print
.global cls
.global putc

cls:
    mov $' ', %al
    mov $WHITE_ON_BLACK, %ah
    mov $VMEM, %ebx
cls_loop:
    mov %ax, (%ebx)

    add $2, %ebx
    cmp $VMEM + MAX_ROWS * MAX_COLS * 2, %ebx
    jne cls_loop

    mov $14, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov $0, %al
    mov $P_SCRDAT, %dx
    out %al, %dx
    mov $15, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov $0, %al
    mov $P_SCRDAT, %dx
    out %al, %dx

    ret

print:
    push %ebp
    mov %esp, %ebp
    mov 8(%ebp), %eax
    xor %ebx, %ebx
print_loop:
    mov (%eax), %bl
    cmp $0, %bx
    je print_end
    inc %eax

    push %eax
    push %bx
    call putc
    pop %bx
    pop %eax

    jmp print_loop
print_end:
    mov %ebp, %esp
    pop %ebp
    ret

putc:
    push %ebp
    mov %esp, %ebp
    
    // get cursor
    xor %ebx, %ebx
    mov $14, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov $P_SCRDAT, %dx
    in %dx, %al
    mov %al, %bh
    mov $15, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov $P_SCRDAT, %dx
    in %dx, %al
    mov %al, %bl

    // ebx is cursor position

    mov 8(%esp), %eax
    ; and $0xff, %eax
    mov $WHITE_ON_BLACK, %ah
    mov $VMEM, %ecx
    lea (%ecx, %ebx, 2), %ecx
    mov %ax, (%ecx)
    inc %bx

    // set cursor to next line

    mov $14, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov %bh, %al
    mov $P_SCRDAT, %dx
    out %al, %dx
    mov $15, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov %bl, %al
    mov $P_SCRDAT, %dx
    out %al, %dx

    mov %ebp, %esp
    pop %ebp
    ret

.equ MAX_COLS, 25
.equ MAX_ROWS, 80
.equ P_SCRCTL, 0x3d4
.equ P_SCRDAT, 0x3d5
.equ VMEM, 0xb8000
.equ WHITE_ON_BLACK, 0x0f
