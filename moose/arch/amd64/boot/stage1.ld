SECTIONS {
     . = 0x7c00;
    .text : {
        *(.text*)
        *(.data*)
        *(.rodata*)
    }

    /* Include fat boot signature */
    .sig : AT(ADDR(.text) + 512 - 2) {
        SHORT(0xaa55);
    }

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
    }
}
