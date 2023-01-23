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
}