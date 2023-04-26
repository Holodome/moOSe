/*
 This file contains code for 'print' procedure 
 that outputs null-terminated string using VGA MMIO
*/
.global print
.global cls
.code32

#define VMEM 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define P_SCRCTL 0x3d4
#define P_SCRDAT 0x3d5
#define WHITE_ON_BLACK 0x0f

cls:
    movb $' ', %al
    movb $WHITE_ON_BLACK, %ah
    movl $VMEM, %ebx
cls_loop:
    movw %ax, (%ebx)

    addl $2, %ebx
    cmpl $VMEM + MAX_ROWS * MAX_COLS * 2, %ebx
    jne cls_loop

    movb $14, %al
    movw $P_SCRCTL, %dx
    outb %al, %dx
    movb $0, %al
    movw $P_SCRDAT, %dx
    outb %al, %dx
    movb $15, %al
    movw $P_SCRCTL, %dx
    outb %al, %dx
    movb $0, %al
    movw $P_SCRDAT, %dx
    outb %al, %dx

    ret

print:
    pushl %ebp
    pushl %ebx
    movl %esp, %ebp

    /* get cursor to ecx */
    xorl %ecx, %ecx
    movb $14, %al
    movw $P_SCRCTL, %dx
    outb %al, %dx
    movw $P_SCRDAT, %dx
    inb %dx, %al
    movb %al, %ch
    movb $15, %al
    movw $P_SCRCTL, %dx
    outb %al, %dx
    movw $P_SCRDAT, %dx
    inb %dx, %al
    movb %al, %cl
    /* ecx is cursor position */
    movl 12(%ebp), %eax
print_loop:
    movb (%eax), %bl
    cmpb $0, %bl
    je print_end
    incl %eax

    movb $WHITE_ON_BLACK, %bh
    movl $VMEM, %edx
    leal (%edx, %ecx, 2), %edx
    movw %bx, (%edx)
    incl %ecx

    jmp print_loop
print_end:
    /* move cursor to next line
     do not bother with scrolling because there should not be much messages
     anyway */
    xorl %edx, %edx
    movl %ecx, %eax
    movl $MAX_COLS, %ebx
    idivl %ebx
    incl %eax
    movl $MAX_COLS, %ebx
    mull %ebx
    movl %eax, %ecx

    /* update cursor */
    movb $14, %al
    movw $P_SCRCTL, %dx
    outb %al, %dx
    movb %ch, %al
    movw $P_SCRDAT, %dx
    outb %al, %dx
    movb $15, %al
    movw $P_SCRCTL, %dx
    outb %al, %dx
    movb %cl, %al
    movw $P_SCRDAT, %dx
    outb %al, %dx

    movl %ebp, %esp
    popl %ebx
    popl %ebp
    retl

