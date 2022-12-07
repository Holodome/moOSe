SECTIONS
{
    . = 0x80000; /* kernel load address for AArch64 */
    
    .text : { 
        KEEP(*(.text.boot))
        *(.text .text.* .gnu.linkonce.t*)
    }

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.d)
    }

    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    _end = .;

    /DISCARD/ : { 
        *(.comment) 
        *(.gnu*) 
        *(.note*) 
        *(.eh_frame*) 
    }
}

__bss_size = (__bss_end - __bss_start) >> 3;
