.extern _kmain
start:
    // zero the bss
    cld
    mov $__bss_start, %edi
    mov $__bss_end, %ecx
    sub %ecx, %edi
    xor %eax, %eax
    shr $2, %ecx
    rep; stosl

    call _kmain
    jmp .
