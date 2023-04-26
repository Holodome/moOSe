.code32
.global kboot
.global kboot64

kboot:
    call enable_long_mode
    jmp .

.code64
kboot64:
    movq %rsp, %rax
    movq $0xffff880000000000, %rbx
    addq %rbx, %rax
    movq %rax, %rbp
    movq %rbp, %rsp

    movq $_end, %rax
    addq %rbx, %rax

    jmp *%rax
