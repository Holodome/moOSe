#include <moose/param.h>

ENTRY(kstart)
SECTIONS {
    . = KERNEL_PHYSICAL_BASE;
    .text : {
        *(.text*)
        *(.data*)
        *(.rodata*)
        . = ALIGN(PAGE_SIZE);
    }
    _end = .;

    /DISCARD/ : {
        *(.comment)
        *(.note.gnu.property)
        *(.eh_frame)
    }
}
