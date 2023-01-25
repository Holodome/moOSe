
start:
    call cls
    push $'a'
    call putc

    push $str
    call print


    jmp .
    call check_has_cpuid
    call check_long_mode_supported

    call _kmain
    jmp .

str: .asciz "hello"

check_has_cpuid:
    pushf
    pop %eax
 
    mov %eax, %ecx
    xor $1 << 21, %eax
 
    push %eax
    popf
 
    pushf
    pop %eax
 
    push %ecx
    popf
 
    xor %ecx, %eax
    jz no_cpuid
    ret
no_cpuid:
    jmp die

check_long_mode_supported:
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb no_long_mode

    mov $0x80000001, %eax
    cpuid
    test $1 << 29, %edx
    jz no_long_mode
    
    ret

no_long_mode:
    jmp die

die: 
    hlt 
    jmp die


