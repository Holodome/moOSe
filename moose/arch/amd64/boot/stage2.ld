ENTRY(load_kernel)
SECTIONS {
    . = 0x1000;
    .text : {
        KEEP(*(.text.entry))
        *(.text*)
        *(.data*)
        *(.rodata*)
        . = ALIGN(512);
    }

    . = ALIGN(4);
    .bss : {
        __bss_start = . ;
        *(.bss*)
        . = ALIGN(4);
        __bss_end = . ;
    }

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
        *(.eh_frame)
    }
}
