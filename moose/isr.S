.extern _isr_handler 
.extern _irq_handler

isr_common_stub:
    // save general purpose registers
    pusha 
    // save ds
    mov %ds, %ax
    push %eax
    // kernel data segment
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // registers_t *
    push %esp
    call isr_handler
    pop %eax

    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa
    add $8, %esp
    iret

irq_common_stub:
    pusha 

    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call irq_handler
    pop %ebx

    pop %ebx
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs

    popa 
    add $8, %esp
    iret

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
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

// 0: Divide By Zero Exception
isr0:
    push $0x00
    push $0x00
    jmp isr_common_stub

// 1: Debug Exception
isr1:
    push $0x00
    push $0x01
    jmp isr_common_stub

// 2: Non Maskable Interrupt Exception
isr2:
    push $0x00
    push $0x02
    jmp isr_common_stub

// 3: Int 3 Exception
isr3:
    push $0x00
    push $0x03
    jmp isr_common_stub

// 4: INTO Exception
isr4:
    push $0x00
    push $0x04
    jmp isr_common_stub

// 5: Out of Bounds Exception
isr5:
    push $0x00
    push $0x05
    jmp isr_common_stub

// 6: Invalid Opcode Exception
isr6:
    push $0x00
    push $0x06
    jmp isr_common_stub

// 7: Coprocessor Not Available Exception
isr7:
    push $0x00
    push $0x07
    jmp isr_common_stub

// 8: Double Fault Exception (With Error Code!)
isr8:
    push $0x08
    jmp isr_common_stub

// 9: Coprocessor Segment Overrun Exception
isr9:
    push $0x00
    push $0x09
    jmp isr_common_stub

// 10: Bad TSS Exception (With Error Code!)
isr10:
    push $0x0a
    jmp isr_common_stub

// 11: Segment Not Present Exception (With Error Code!)
isr11:
    push $0x0b
    jmp isr_common_stub

// 12: Stack Fault Exception (With Error Code!)
isr12:
    push $0x0c
    jmp isr_common_stub

// 13: General Protection Fault Exception (With Error Code!)
isr13:
    push $0x0d
    jmp isr_common_stub

// 14: Page Fault Exception (With Error Code!)
isr14:
    push $0x0e
    jmp isr_common_stub

// 15: Reserved Exception
isr15:
    push $0x00
    push $0x0f
    jmp isr_common_stub

// 16: Floating Point Exception
isr16:
    push $0x00
    push $0x10
    jmp isr_common_stub

// 17: Alignment Check Exception
isr17:
    push $0x00
    push $0x11
    jmp isr_common_stub

// 18: Machine Check Exception
isr18:
    push $0x00
    push $0x12
    jmp isr_common_stub

// 19: Reserved
isr19:
    push $0x00
    push $0x13
    jmp isr_common_stub

// 20: Reserved
isr20:
    push $0x00
    push $0x14
    jmp isr_common_stub

// 21: Reserved
isr21:
    push $0x00
    push $0x15
    jmp isr_common_stub

// 22: Reserved
isr22:
    push $0x00
    push $0x16
    jmp isr_common_stub

// 23: Reserved
isr23:
    push $0x00
    push $0x17
    jmp isr_common_stub

// 24: Reserved
isr24:
    push $0x00
    push $0x18
    jmp isr_common_stub

// 25: Reserved
isr25:
    push $0x00
    push $0x19
    jmp isr_common_stub

// 26: Reserved
isr26:
    push $0x00
    push $0x1a
    jmp isr_common_stub

// 27: Reserved
isr27:
    push $0x00
    push $0x1b
    jmp isr_common_stub

// 28: Reserved
isr28:
    push $0x00
    push $0x1c
    jmp isr_common_stub

// 29: Reserved
isr29:
    push $0x00
    push $0x1d
    jmp isr_common_stub

// 30: Reserved
isr30:
    push $0x00
    push $0x1e
    jmp isr_common_stub

// 31: Reserved
isr31:
    push $0x00
    push $0x1f
    jmp isr_common_stub

.global irq0
.global irq1
.global irq2
.global irq3
.global irq4
.global irq5
.global irq6
.global irq7
.global irq8
.global irq9
.global irq10
.global irq11
.global irq12
.global irq13
.global irq14
.global irq15

// IRQ handlers
irq0:
	push $0x00
	push $0x20
	jmp irq_common_stub

irq1:
	push $0x01
	push $0x21
	jmp irq_common_stub

irq2:
	push $0x02
	push $0x22
	jmp irq_common_stub

irq3:
	push $0x03
	push $0x23
	jmp irq_common_stub

irq4:
	push $0x04
	push $0x24
	jmp irq_common_stub

irq5:
	push $0x05
	push $0x25
	jmp irq_common_stub

irq6:
	push $0x06
	push $0x26
	jmp irq_common_stub

irq7:
	push $0x07
	push $0x27
	jmp irq_common_stub

irq8:
	push $0x08
	push $0x28
	jmp irq_common_stub

irq9:
	push $0x09
	push $0x29
	jmp irq_common_stub

irq10:
	push $0x0a
	push $0x2a
	jmp irq_common_stub

irq11:
	push $0x0b
	push $0x2b
	jmp irq_common_stub

irq12:
	push $0x0c
	push $0x2c
	jmp irq_common_stub

irq13:
	push $0x0d
	push $0x2d
	jmp irq_common_stub

irq14:
	push $0x0e
	push $0x2e
	jmp irq_common_stub

irq15:
	push $0x0f
	push $0x2f
	jmp irq_common_stub
