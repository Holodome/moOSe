/* 
 Long mode (64-bit) enabling related procedures
*/
.code32
.global enable_long_mode

enable_long_mode:
    /* first set up paging */

    /* clear the page tables */
    mov $0x1000, %edi
    mov %edi, %cr3
    xor %eax, %eax
    mov $8192, %ecx
    rep stosl
    mov %cr3, %edi

    /* set up page tables */
    movl $0x2003, (%edi)
    add $0x1000, %edi
    movl $0x3003, (%edi)
    add $0x1000, %edi
    movl $0x4003, (%edi)
    add $0x1000, %edi

    /* set up low page table entries with identity mapping */
    mov $3, %ebx
    mov $512, %ecx
set_low_entry:
    movl %ebx, (%edi)
    add $0x1000, %ebx
    add $8, %edi
    loop set_low_entry

    /* (0xffff880000000000 >> 39) & 0x1ff) = 272 */
    movl $(0x1000 + 272*8), %edi
    movl $0x5003, (%edi)
    movl $0x5000, %edi
    movl $0x6003, (%edi)
    add $0x1000, %edi
    movl $0x7003, (%edi)
    add $0x1000, %edi

    /* set up high page table entries */
    mov $3, %ebx
    mov $512, %ecx
set_high_entry:
    movl %ebx, (%edi)
    add $0x1000, %ebx
    add $8, %edi
    loop set_high_entry

    mov %cr4, %eax
    or $1<<5, %eax
    mov %eax, %cr4

    /* now paging is set up, time to enable it */

    mov $0xc0000080, %ecx
    rdmsr
    or $1<<8, %eax
    wrmsr
    
    mov %cr0, %eax
    or $1<<31 | 1, %eax
    mov %eax, %cr0

    /* now we are at long mode, but in 32-bit compatibility mode
     load new gdt */

    lgdt gdt_descriptor
    ljmp $0x8, $amd64

/* access bits */
#define AB_PRESENT  (1 << 7)
#define AB_NOT_SYS  (1 << 4)
#define AB_EXEC     (1 << 3)
#define AB_DC       (1 << 2)
#define AB_RW       (1 << 1)
#define AB_ACCESSED (1 << 0)

/* flag bits */
#define FB_GRAN4K    (1 << 7)
#define FB_SZ32      (1 << 6)
#define FB_LONG_MODE (1 << 5)

gdt_start:
    .quad 0x0

gdt_code:
    .long 0xffff
    .byte 0x00
    .byte AB_PRESENT | AB_NOT_SYS | AB_EXEC | AB_RW
    .byte FB_GRAN4K | FB_LONG_MODE | 0x0f
    .byte 0x00

gdt_data:
    .long 0xffff
    .byte 0x00
    .byte AB_PRESENT | AB_NOT_SYS | AB_RW
    .byte FB_GRAN4K | FB_SZ32 | 0x0f
    .byte 0x00

gdt_tss:
    .long 0x00000068
    .long 0x00CF8900

gdt_end:
gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start

.code64
amd64:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    jmp kboot64
