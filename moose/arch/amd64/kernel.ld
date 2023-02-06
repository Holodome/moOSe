ENTRY(kmain)
SECTIONS {
    /* kernel virtual base + kernel offset in physical + kboot.S */
    . = 0xffff880000000000 + 0x100000 + 0x1000; 

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
