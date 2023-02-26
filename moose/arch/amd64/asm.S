/* this is junkyard for all functions that require direct assembly and
  are to big to be readable in inline assembly */
.global get_registers
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

bootstrap_task:
    movq %rdi, %rsp
    pushq %rsi
    retq

