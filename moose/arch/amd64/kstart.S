.global kstart 

#include <param.h>

.section ".text.kstart"
kstart:
    movq %rsp, %rbp
    sgdt -10(%rbp)
    movq $PHYSMEM_VIRTUAL_BASE, %rbx
    addq %rbx, -8(%rbp)
    lgdt -10(%rbp)

    call kmain
    jmp .
