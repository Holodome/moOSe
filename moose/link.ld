SECTIONS {
    . = 0x1000;
    .text : {
        *(.text)
    }
    __bss_start = .;
    .bss : {
        *(.bss)
    }
    __bss_end = .;
}
