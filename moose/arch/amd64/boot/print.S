//
// This file contains code for 'print' procedure 
// that outputs null-terminated string using VGA MMIO
//
.global print
.global cls
.code32

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
    push %ebx
    mov %esp, %ebp

    // get cursor to ecx
    xor %ecx, %ecx
    mov $14, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov $P_SCRDAT, %dx
    in %dx, %al
    mov %al, %ch
    mov $15, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov $P_SCRDAT, %dx
    in %dx, %al
    mov %al, %cl
    // ecx is cursor position
    mov 12(%ebp), %eax
print_loop:
    mov (%eax), %bl
    cmp $0, %bl
    je print_end
    inc %eax

    mov $WHITE_ON_BLACK, %bh
    mov $VMEM, %edx
    lea (%edx, %ecx, 2), %edx
    mov %bx, (%edx)
    inc %ecx

    jmp print_loop
print_end:
    // move cursor to next line
    // do not bother with scrolling because there should not be much messages
    // anyway
    xor %edx, %edx
    mov %ecx, %eax
    mov $MAX_COLS, %ebx
    idivl %ebx
    inc %eax
    mov $MAX_COLS, %ebx
    mul %ebx
    mov %eax, %ecx

    // update cursor
    mov $14, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov %ch, %al
    mov $P_SCRDAT, %dx
    out %al, %dx
    mov $15, %al
    mov $P_SCRCTL, %dx
    out %al, %dx
    mov %cl, %al
    mov $P_SCRDAT, %dx
    out %al, %dx

    mov %ebp, %esp
    pop %ebx
    pop %ebp
    ret

.equ MAX_ROWS, 25
.equ MAX_COLS, 80
.equ P_SCRCTL, 0x3d4
.equ P_SCRDAT, 0x3d5
.equ VMEM, 0xb8000
.equ WHITE_ON_BLACK, 0x0f
