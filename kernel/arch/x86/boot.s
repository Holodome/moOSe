.att_syntax
.code16
.global _start

.text

_start:
    mov $0x21, %al
    mov $0x0e, %ah
    mov $0x00, %bh
    mov $0x07, %bl

    int $0x10

halt:
    jmp halt

.org 510
.word 0xaa55

