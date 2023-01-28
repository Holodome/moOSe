ENTRY(kstart)
SECTIONS {
    . = 0x100000;
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
}
