#include <param.h>

ENTRY(kmain)
SECTIONS {
    /* kernel virtual base + kernel offset in physical + kboot.S */
    . = KERNEL_VIRTUAL_BASE + KBOOT_SIZE; 

    __start = .;
    __text = .;
    .text : {
        KEEP(*(.text.kstart))
        *(.text*)
    }
    . = ALIGN(16);
    __etext = .;

    . = ALIGN(16);
    __data = .;
    .data : {
        *(.data*)
        *(.rodata*)
    }
    . = ALIGN(16);
    __edata = .;

    . = ALIGN(16);
    __bss_start = . ;
    .bss : {
        *(.bss*)
    }
    . = ALIGN(16);
    __bss_end = . ;
    __end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
        *(.eh_frame)
    }
}
