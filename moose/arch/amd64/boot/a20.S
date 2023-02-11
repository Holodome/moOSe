/*
 A20 line (21-st bit of addressing) is disabled by default on most intel
 architectures (qemu does enable it, however).
 If one wishes to proceed into 32-bit and 64-bit addressing, they need to 
 enable it.

Algorithm:
Test if A20 is already enabled - if it is you don't need to do anything at all
Try the BIOS function. Ignore the returned status.
Test if A20 is enabled (to see if the BIOS function actually worked or not)
Try the keyboard controller method.
Test if A20 is enabled in a loop with a time-out (as the keyboard controller method may work slowly)
Try the Fast A20 method last
Test if A20 is enabled in a loop with a time-out (as the fast A20 method may work slowly)
If none of the above worked, give up

Note that enalbing only with 8042 is implemented
*/
.code32
.global enable_a20

enable_a20:
    call check_a20_enabled
    jc 1f
    ret
1:
    /* try the 8042 method */
    call wait_8042_out
    mov $0xad, %al
    out %al, $0x64

    call wait_8042_out
    mov $0xd0, %al
    out %al, $0x64

    call wait_8042_in
    in $0x60, %al
    mov %al, %bl

    call wait_8042_out
    mov $0xd1, %al
    out %al, $0x64

    call wait_8042_out
    mov %bl, %al
    or $2, %al
    out %al, $0x60

    call wait_8042_out
    mov $0xae, %al
    out %al, $0x64

    call wait_8042_out

1:
    mov $128, %ecx
    call check_a20_enabled
    jnc 2f
    loop 1b
    jmp 3f
2:
    ret
3:
    /* 8042 did not work */
    stc
    ret

/* check if a20 is enabled and store result to carry flag */
check_a20_enabled:
    pusha 
    mov $0x112345, %edi
    mov $0x012345, %esi
    mov %esi, (%esi)
    mov %edi, (%edi)
    cmpsl
    popa
    jne 1f
    stc
    ret
1:  /* a20 is on */
    clc
    ret

/* wait until 8042 PS/2 controller output is ready */
wait_8042_out:
    in $0x64, %al
    test $1, %al
    jnz wait_8042_out
    ret

/* wait until 8042 PS/2 controller input is ready */
wait_8042_in:
    in $0x64, %al
    test $2, %al
    jnz wait_8042_in
    ret
