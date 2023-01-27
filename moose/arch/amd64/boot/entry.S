.code32

start:
    call cls
    push $intro_msg
    call print
    sub $4, %esp

    call enable_a20
    test %eax, %eax
    jne no_a20

    call check_has_cpuid
    call check_long_mode_supported

    mov $0x100000, %edi
    mov $1, %ebp
    mov $2, %esi
    call ata_read_sectors
    jc read_fail

    push $0x100000 + 239
    call print

    jmp .

    push $switching_to_long_msg
    call print
    sub $4, %esp

    jmp enable_long_mode

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
    sub $4, %esp
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
    sub $4, %esp
    jmp die

no_a20:
    push $no_a20_msg
    call print
    sub $4, %esp
    jmp die

read_fail:
    push $failed_to_read
    call print
    sub $4, %esp
    jmp die

die: 
    hlt 
    jmp die

.section ".rodata"
intro_msg: .asciz "entered protected mode"
no_cpuid_msg: .asciz "cpu has no cpuid instruction"
no_long_mode_msg: .asciz "cpu does not support long mode"
switching_to_long_msg: .asciz "switching to long mode"
no_a20_msg: .asciz "failed to enable a20 line",
failed_to_read: .asciz "failed to read disk"