ENTRY(kstart)
SECTIONS {
    . = 0x100000;
    .text : {
        *(.text*)
        *(.data*)
        *(.rodata*)
        . = ALIGN(4096);
    }
    _end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
        *(.eh_frame)
    }
}
