SECTIONS {
    .text : {
        *(.boot1)
        *(.boot1.sig)
        *(.boot2)

        *(.text)
        *(.data)
        *(.rodata)
    }
}