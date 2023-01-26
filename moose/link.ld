SECTIONS {
    . = 0x1000;
    .text : {
        *(.text)
        *(.data)
        *(.rodata)
        *(.rodata.*)
    }
    __bss_start = . ;
    .bss : {
        *(.bss)
        *(.bss.*)
    }
    . = ALIGN(4);
    __bss_end = . ;
}
