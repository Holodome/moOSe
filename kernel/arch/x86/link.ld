/* FIXME: This seems to be architecture and media-dependent */
SECTIONS {
    . = 0x7c00;
    .boot1 : {
        KEEP(*(.boot1.text))
    }

    /* Include fat boot signature */
    .boot1.sig : AT(ADDR(.boot1) + 512 - 2) {
      SHORT(0xaa55);
    }

    . = 0x7e00;
    .boot2 : {
        *(.boot2.text)
    }
}
