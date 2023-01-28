.code32
.global kstart
.global kstart64

kstart:
    call enable_long_mode
    jmp .

.code64
kstart64:
    jmp .
