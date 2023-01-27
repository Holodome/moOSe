SECTIONS {
    . = 0x1000;
    .text : {
        *(.text*)
        *(.data*)
        *(.rodata*)
        . = ALIGN(512);
    }
}
