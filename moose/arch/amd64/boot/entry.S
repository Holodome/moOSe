.global start32

.section ".text.entry"
.code16
start:
    call get_memory_map
    jmp switch_to_32bit

.code32
start32:
    movl $__bss_start, %edi
    movl $__bss_end, %ecx
    xorl %eax, %eax
    subl %edi, %ecx
    shrl $2, %ecx
    rep stosl

    call cls

    call enable_a20
    jc no_a20

    call check_has_cpuid
    call check_long_mode_supported

    call load_kernel
    testl %eax, %eax
    jne read_fail
    
    ljmp $0x8, $0x100000
check_has_cpuid:
    pushf
    popl %eax
 
    movl %eax, %ecx
    xorl $1 << 21, %eax
 
    pushl %eax
    popfl
 
    pushfl
    popl %eax
 
    pushl %ecx
    popfl
 
    xorl %ecx, %eax
    jz no_cpuid
    ret

no_cpuid:
    pushl $no_cpuid_msg
    call print
    addl $4, %esp
    jmp die

check_long_mode_supported:
    movl $0x80000000, %eax
    cpuid
    cmpl $0x80000001, %eax
    jb no_long_mode

    movl $0x80000001, %eax
    cpuid
    testl $1 << 29, %edx
    jz no_long_mode_msg
    ret

no_long_mode:
    pushl $no_long_mode_msg
    call print
    jmp die

no_a20:
    pushl $no_a20_msg
    call print
    jmp die

read_fail:
    pushl $failed_to_read
    call print
    jmp die

die: 
    hlt 
    jmp die

.section ".rodata"
no_cpuid_msg: .asciz "cpu has no cpuid instruction"
no_long_mode_msg: .asciz "cpu does not support long mode"
no_a20_msg: .asciz "failed to enable a20 line",
failed_to_read: .asciz "failed to load kernel from disk"
