.code32
.global kstart

kstart:
    call enable_long_mode
    jmp .
