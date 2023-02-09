/* Get memory map using bios E820 int 0x15 
   Store it at constant address to that OS can look it up 
*/
.global get_memory_map
.code16

#define COUNT_ADDR 0x500
#define DEST_ADDR 0x504

get_memory_map:
    mov $DEST_ADDR, %di
    xor %ebx, %ebx
    mov $0x534d4150, %edx
    mov $COUNT_ADDR, %ebp
    movl $0, (%ebp)
1:
    mov $0xe820, %eax
    mov $24, %ecx
    int $0x15
    jc 2f
    incl (%ebp)
    test %ebx, %ebx
    jz 2f
    add $24, %edi
    jmp 1b
2:
    ret
