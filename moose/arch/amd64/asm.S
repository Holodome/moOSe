/* this is junkyard for all functions that require direct assembly and
  are to big to be readable in inline assembly */
.global get_registers
.global set_stack
.global context_switch
.global bootstrap_task

get_registers:
    pushq %rax
    movq %rdi, %rax
    movq %rdi, 0x0(%rax)
    movq %rsi, 0x8(%rax)
    movq %rbp, 0x10(%rax)
    movq %rsp, 0x18(%rax)
    movq %rbx, 0x20(%rax)
    movq %rdx, 0x28(%rax)
    movq %rcx, 0x30(%rax)
    movq %rax, 0x38(%rax)
    movq %r8, 0x40(%rax)
    movq %r9, 0x48(%rax)
    movq %r10, 0x50(%rax)
    movq %r11, 0x58(%rax)
    movq %r12, 0x60(%rax)
    movq %r13, 0x68(%rax)
    movq %r14, 0x70(%rax)
    movq %r15, 0x78(%rax)
    pushq %rbx
    leaq 0(%rip), %rbx
    movq %rbx, 0x80(%rax)
    pushfq
    popq %rbx
    movq %rbx, 0x88(%rax)
    movq %cr0, %rbx
    movq %rbx, 0x90(%rax)
    movq %cr2, %rbx
    movq %rbx, 0x98(%rax)
    movq %cr3, %rbx
    movq %rbx, 0xa0(%rax)
    popq %rbx
    popq %rax
    retq

set_stack:
    movq %rsi, %rcx
    movq %rsp, %rsi
    subq %rsi, %rcx
    subq %rcx, %rdi
    movq %rdi, %rbx
    rep movsb
    movq %rbx, %rsp
    movq %rsp, %rbp
    retq

context_switch:
    movq %rdi, %rax
    movq %rsi, %rdx
    /* save system V ABI preserved general-purpose registers */
    pushq %rbx
    pushq %rsp
    pushq %rbp
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    /* save flags and rbp */
    pushfq
    pushq %rbp
    /* save esp and eip */
    movq %rsp, (%rax)
    leaq 1f(%rip), %rdi
    mov %rdi, 8(%rax)
    /* "movq 1f, 8(%rax)\n" */
    /* switch stack */
    movq (%rdx), %rsp
    /* 'call' new thread */
    pushq 8(%rdx)
    ret
1:
    popq %rbp
    popfq
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rbp
    popq %rsp
    popq %rbx
    ret

bootstrap_task:
    movq (%rdi), %rsp
    pushq 8(%rdi)
    retq
