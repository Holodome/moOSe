.global kstart 
.section ".text.kstart"

#include <kernel.h>

kstart:
    mov %rsp, %rbp
    sgdt -10(%rbp)
    mov $PHYSMEM_VIRTUAL_BASE, %rbx
    add %rbx, -8(%rbp)
    lgdt -10(%rbp)

    call kmain
    jmp .
