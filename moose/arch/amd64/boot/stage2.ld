SECTIONS {
    . = 0x1000;
    .text : {
        *(.text*)
        *(.data*)
        *(.rodata*)
    }

    . = ALIGN(4);
    __bss_start = . ;
    .bss : {
        *(.bss*)
    }
    . = ALIGN(4);
    __bss_end = . ;
}
