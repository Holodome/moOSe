isr_common_stub:
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rax
    pushq %rcx
    pushq %rdx
    pushq %rbx
    pushq %rsp
    pushq %rbp
    pushq %rsi
    pushq %rdi

    mov %rsp, %rdi
    call isr_handler

    popq %rdi
    popq %rsi
    popq %rbp
    popq %rax
    popq %rbx
    popq %rdx
    popq %rcx
    popq %rax
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15

    /* account for pushed isr_number, exception_code */
    add $16, %rsp
    iretq

.global exception0
.global exception1
.global exception2
.global exception3
.global exception4
.global exception5
.global exception6
.global exception7
.global exception8
.global exception9
.global exception10
.global exception11
.global exception12
.global exception13
.global exception14
.global exception15
.global exception16
.global exception17
.global exception18
.global exception19
.global exception20
.global exception21
.global exception22
.global exception23
.global exception24
.global exception25
.global exception26
.global exception27
.global exception28
.global exception29
.global exception30
.global exception31

/* 0: Divide By Zero Exception */
exception0:
    pushq $0x00
    pushq $0x00
    jmp isr_common_stub

/* 1: Debug Exception */
exception1:
    pushq $0x00
    pushq $0x01
    jmp isr_common_stub

/* 2: Non Maskable Interrupt Exception */
exception2:
    pushq $0x00
    pushq $0x02
    jmp isr_common_stub

/* 3: Int 3 Exception */
exception3:
    pushq $0x00
    pushq $0x03
    jmp isr_common_stub

/* 4: INTO Exception */
exception4:
    pushq $0x00
    pushq $0x04
    jmp isr_common_stub

/* 5: Out of Bounds Exception */
exception5:
    pushq $0x00
    pushq $0x05
    jmp isr_common_stub

/* 6: Invalid Opcode Exception */
exception6:
    pushq $0x00
    pushq $0x06
    jmp isr_common_stub

/* 7: Coprocessor Not Available Exception */
exception7:
    pushq $0x00
    pushq $0x07
    jmp isr_common_stub

/* 8: Double Fault Exception (With Error Code!) */
exception8:
    pushq $0x08
    jmp isr_common_stub

/* 9: Coprocessor Segment Overrun Exception */
exception9:
    pushq $0x00
    pushq $0x09
    jmp isr_common_stub

/* 10: Bad TSS Exception (With Error Code!) */
exception10:
    pushq $0x0a
    jmp isr_common_stub

/* 11: Segment Not Present Exception (With Error Code!) */
exception11:
    pushq $0x0b
    jmp isr_common_stub

/* 12: Stack Fault Exception (With Error Code!) */
exception12:
    pushq $0x0c
    jmp isr_common_stub

/* 13: General Protection Fault Exception (With Error Code!) */
exception13:
    pushq $0x0d
    jmp isr_common_stub

/* 14: Page Fault Exception (With Error Code!) */
exception14:
    pushq $0x0e
    jmp isr_common_stub

/* 15: Reserved Exception */
exception15:
    pushq $0x00
    pushq $0x0f
    jmp isr_common_stub

/* 16: Floating Point Exception */
exception16:
    pushq $0x00
    pushq $0x10
    jmp isr_common_stub

/* 17: Alignment Check Exception */
exception17:
    pushq $0x00
    pushq $0x11
    jmp isr_common_stub

/* 18: Machine Check Exception */
exception18:
    pushq $0x00
    pushq $0x12
    jmp isr_common_stub

/* 19: Reserved */
exception19:
    pushq $0x00
    pushq $0x13
    jmp isr_common_stub

/* 20: Reserved */
exception20:
    pushq $0x00
    pushq $0x14
    jmp isr_common_stub

/* 21: Reserved */
exception21:
    pushq $0x00
    pushq $0x15
    jmp isr_common_stub

/* 22: Reserved */
exception22:
    pushq $0x00
    pushq $0x16
    jmp isr_common_stub

/* 23: Reserved */
exception23:
    pushq $0x00
    pushq $0x17
    jmp isr_common_stub

/* 24: Reserved */
exception24:
    pushq $0x00
    pushq $0x18
    jmp isr_common_stub

/* 25: Reserved */
exception25:
    pushq $0x00
    pushq $0x19
    jmp isr_common_stub

/* 26: Reserved */
exception26:
    pushq $0x00
    pushq $0x1a
    jmp isr_common_stub

/* 27: Reserved */
exception27:
    pushq $0x00
    pushq $0x1b
    jmp isr_common_stub

/* 28: Reserved */
exception28:
    pushq $0x00
    pushq $0x1c
    jmp isr_common_stub

/* 29: Reserved */
exception29:
    pushq $0x00
    pushq $0x1d
    jmp isr_common_stub

/* 30: Reserved */
exception30:
    pushq $0x00
    pushq $0x1e
    jmp isr_common_stub

/* 31: Reserved */
exception31:
    pushq $0x00
    pushq $0x1f
    jmp isr_common_stub

.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15

/* isr handlers */
isr0:
	pushq $0x00
	pushq $0x20
	jmp isr_common_stub

isr1:
	pushq $0x01
	pushq $0x21
	jmp isr_common_stub

isr2:
	pushq $0x02
	pushq $0x22
	jmp isr_common_stub

isr3:
	pushq $0x03
	pushq $0x23
	jmp isr_common_stub

isr4:
	pushq $0x04
	pushq $0x24
	jmp isr_common_stub

isr5:
	pushq $0x05
	pushq $0x25
	jmp isr_common_stub

isr6:
	pushq $0x06
	pushq $0x26
	jmp isr_common_stub

isr7:
	pushq $0x07
	pushq $0x27
	jmp isr_common_stub

isr8:
	pushq $0x08
	pushq $0x28
	jmp isr_common_stub

isr9:
	pushq $0x09
	pushq $0x29
	jmp isr_common_stub

isr10:
	pushq $0x0a
	pushq $0x2a
	jmp isr_common_stub

isr11:
	pushq $0x0b
	pushq $0x2b
	jmp isr_common_stub

isr12:
	pushq $0x0c
	pushq $0x2c
	jmp isr_common_stub

isr13:
	pushq $0x0d
	pushq $0x2d
	jmp isr_common_stub

isr14:
	pushq $0x0e
	pushq $0x2e
	jmp isr_common_stub

isr15:
	pushq $0x0f
	pushq $0x2f
	jmp isr_common_stub
