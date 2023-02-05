ENTRY(kstart)
SECTIONS {
    .text : {
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
