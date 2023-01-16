/* FIXME: This seems to be architecture and media-dependent */
SECTIONS {
    . = 0x7c00;
    .text : {
        *(.text)
        *(.text.*)
        *(.data)
        *(.rodata)
    }

    /* Include fat boot signature */
    .sig : AT(ADDR(.text) + 512 - 2) {
      SHORT(0xaa55);
    }

    __stack_bottom = .;
    . = . + 0x1000;
    __stack_top = .;
}
