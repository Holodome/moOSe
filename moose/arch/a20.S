//
// A20 line (21-st bit of addressing) is disabled by default on most intel
// architectures (qemu does enable it, however).
// If one wishes to proceed into 32-bit and 64-bit addressing, they need to 
// enable it.
//
.code32
.global enable_a20

enable_a20:
    // first, check if a20 is already enabled
    pusha 
    mov $0x112345, %edi
    mov $0x012345, %esi
    mov %esi, (%esi)
    mov %edi, (%edi)
    cmps
    popa
    je 1f
    mov $0, %eax
    ret
1:  // a20 is off
    // TODO: implement algorithm for enable a20 from osdev wiki
    // by trying different methods
    mov $1, %eax
    ret

