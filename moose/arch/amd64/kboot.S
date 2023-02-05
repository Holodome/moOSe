.code32
.global kboot
.global kboot64

kboot:
    call enable_long_mode
    jmp .

.code64
kboot64:
    mov $_end, %rax
    mov $0xffff880000000000, %rbx
    add %rbx, %rax

    jmp *%rax
halt:
    cli
    hlt
    jmp halt

