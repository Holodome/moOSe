.code32
.global kstart
.global kstart64

kstart:
    call enable_long_mode
    jmp .

.code64
kstart64:
    mov $__bss_start, %rdi
    mov $__bss_end, %rcx
    xor %rax, %rax
    sub %rdi, %rcx
    shr $3, %rcx
    rep stosq

    call kmain
halt:
    cli
    hlt
    jmp halt

