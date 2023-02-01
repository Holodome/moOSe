.global start32

.code16
start:
    call get_memory_map
    jmp switch_to_32bit
.code32
start32:
    mov $__bss_start, %edi
    mov $__bss_end, %ecx
    xor %eax, %eax
    sub %edi, %ecx
    shr $2, %ecx
    rep stosl

    call cls
    call enable_a20
    test %eax, %eax
    jne no_a20

    call check_has_cpuid
    call check_long_mode_supported

    call load_kernel
    test %eax, %eax
    jne read_fail
    
    ljmp $0x8, $0x100000
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
    push $no_cpuid_msg
    call print
    add $4, %esp
    jmp die

check_long_mode_supported:
    mov $0x80000000, %eax
    cpuid
    cmp $0x80000001, %eax
    jb no_long_mode

    mov $0x80000001, %eax
    cpuid
    test $1 << 29, %edx
    jz no_long_mode_msg
    ret

no_long_mode:
    push $no_long_mode_msg
    call print
    add $4, %esp
    jmp die

no_a20:
    push $no_a20_msg
    call print
    add $4, %esp
    jmp die

read_fail:
    push $failed_to_read
    call print
    add $4, %esp
    jmp die

die: 
    hlt 
    jmp die

.section ".rodata"
no_cpuid_msg: .asciz "cpu has no cpuid instruction"
no_long_mode_msg: .asciz "cpu does not support long mode"
no_a20_msg: .asciz "failed to enable a20 line",
failed_to_read: .asciz "failed to read disk"
