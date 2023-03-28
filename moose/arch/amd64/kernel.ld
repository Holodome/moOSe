#include <param.h>

ENTRY(kmain)
SECTIONS {
    /* kernel virtual base + kernel offset in physical + kboot.S */
    . = KERNEL_VIRTUAL_BASE + KBOOT_SIZE; 

    .text : {
        KEEP(*(.text.kstart))
        *(.text*)
        *(.data*)
        *(.rodata*)
    }

    . = ALIGN(8);
    .bss : {
        __bss_start = . ;
        *(.bss*)
        . = ALIGN(8);
        __bss_end = . ;
    }

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
        *(.eh_frame)
    }
}
