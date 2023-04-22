#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>
#include <arch/cpu.h>
#include <assert.h>
#include <kstdio.h>
#include <panic.h>

// clang-format off
#define ENUMERATE_ISRS                                                         \
    _ISR(32) _ISR(33) _ISR(34) _ISR(35) _ISR(36) _ISR(37) _ISR(38) _ISR(39)    \
    _ISR(40) _ISR(41) _ISR(42) _ISR(43) _ISR(44) _ISR(45) _ISR(46) _ISR(47)    \
    _ISR(48) _ISR(49) _ISR(50) _ISR(51) _ISR(52) _ISR(53) _ISR(54) _ISR(55)    \
    _ISR(56) _ISR(57) _ISR(58) _ISR(59) _ISR(60) _ISR(61) _ISR(62) _ISR(63)    \
    _ISR(64) _ISR(65) _ISR(66) _ISR(67) _ISR(68) _ISR(69) _ISR(70) _ISR(71)    \
    _ISR(72) _ISR(73) _ISR(74) _ISR(75) _ISR(76) _ISR(77) _ISR(78) _ISR(79)    \
    _ISR(80) _ISR(81) _ISR(82) _ISR(83) _ISR(84) _ISR(85) _ISR(86) _ISR(87)    \
    _ISR(88) _ISR(89) _ISR(90) _ISR(91) _ISR(92) _ISR(93) _ISR(94) _ISR(95)    \
    _ISR(96) _ISR(97) _ISR(98) _ISR(99) _ISR(100) _ISR(101) _ISR(102) _ISR(103)    \
    _ISR(104) _ISR(105) _ISR(106) _ISR(107) _ISR(108) _ISR(109) _ISR(110) _ISR(111)    \
    _ISR(112) _ISR(113) _ISR(114) _ISR(115) _ISR(116) _ISR(117) _ISR(118) _ISR(119)    \
    _ISR(120) _ISR(121) _ISR(122) _ISR(123) _ISR(124) _ISR(125) _ISR(126) _ISR(127)    \
    _ISR(128) _ISR(129) _ISR(130) _ISR(131) _ISR(132) _ISR(133) _ISR(134) _ISR(135)    \
    _ISR(136) _ISR(137) _ISR(138) _ISR(139) _ISR(140) _ISR(141) _ISR(142) _ISR(143)    \
    _ISR(144) _ISR(145) _ISR(146) _ISR(147) _ISR(148) _ISR(149) _ISR(150) _ISR(151)    \
    _ISR(152) _ISR(153) _ISR(154) _ISR(155) _ISR(156) _ISR(157) _ISR(158) _ISR(159)    \
    _ISR(160) _ISR(161) _ISR(162) _ISR(163) _ISR(164) _ISR(165) _ISR(166) _ISR(167)    \
    _ISR(168) _ISR(169) _ISR(170) _ISR(171) _ISR(172) _ISR(173) _ISR(174) _ISR(175)    \
    _ISR(176) _ISR(177) _ISR(178) _ISR(179) _ISR(180) _ISR(181) _ISR(182) _ISR(183)    \
    _ISR(184) _ISR(185) _ISR(186) _ISR(187) _ISR(188) _ISR(189) _ISR(190) _ISR(191)    \
    _ISR(192) _ISR(193) _ISR(194) _ISR(195) _ISR(196) _ISR(197) _ISR(198) _ISR(199)    \
    _ISR(200) _ISR(201) _ISR(202) _ISR(203) _ISR(204) _ISR(205) _ISR(206) _ISR(207)    \
    _ISR(208) _ISR(209) _ISR(210) _ISR(211) _ISR(212) _ISR(213) _ISR(214) _ISR(215)    \
    _ISR(216) _ISR(217) _ISR(218) _ISR(219) _ISR(220) _ISR(221) _ISR(222) _ISR(223)    \
    _ISR(224) _ISR(225) _ISR(226) _ISR(227) _ISR(228) _ISR(229) _ISR(230) _ISR(231)    \
    _ISR(232) _ISR(233) _ISR(234) _ISR(235) _ISR(236) _ISR(237) _ISR(238) _ISR(239)    \
    _ISR(240) _ISR(241) _ISR(242) _ISR(243) _ISR(244) _ISR(245) _ISR(246) _ISR(247)    \
    _ISR(248) _ISR(249) _ISR(250) _ISR(251) _ISR(252) _ISR(253) _ISR(254) _ISR(255)

// clang-format on

// Port address for master PIC
#define PIC1 0x20
// Port address for slave PIC
#define PIC2 0xA0
#define PIC1_CMD PIC1
#define PIC1_DAT (PIC1 + 1)
#define PIC2_CMD PIC2
#define PIC2_DAT (PIC2 + 1)

#define PIC_EOI 0x20 /* End-of-interrupt command code */
#define IRQ_BASE 32

#define EXCEPTION_PAGE_FAULT 0xe

static struct idt_entry idt[256] __aligned(16);

static void set_idt_entry(u8 n, u64 isr_addr) {
    struct idt_entry *e = idt + n;
    e->offset_low = isr_addr & 0xffff;
    e->selector = 0x08;
    e->ist = 0;
    e->attr = 0x8e;
    e->offset_mid = (isr_addr >> 16) & 0xffff;
    e->offset_high = isr_addr >> 32;
    e->reserved = 0;
}

static void load_idt(void) {
    struct idt_reg idt_reg;
    idt_reg.offset = (u64)&idt[0];
    idt_reg.size = 256 * sizeof(struct idt_entry) - 1;
    asm volatile("lidt %0\n" : : "m"(idt_reg));
}

void eoi(u8 irq) {
    if (irq >= 8 + IRQ_BASE)
        port_out8(PIC2_CMD, PIC_EOI);

    port_out8(PIC1_CMD, PIC_EOI);
}

__used __noinline __naked void isr_common_stub(void) {
    asm volatile("pushq %r15\n"
                 "pushq %r14\n"
                 "pushq %r13\n"
                 "pushq %r12\n"
                 "pushq %r11\n"
                 "pushq %r10\n"
                 "pushq %r9\n"
                 "pushq %r8\n"
                 "pushq %rax\n"
                 "pushq %rcx\n"
                 "pushq %rdx\n"
                 "pushq %rbx\n"
                 "pushq %rsp\n"
                 "pushq %rbp\n"
                 "pushq %rsi\n"
                 "pushq %rdi\n"
                 "movq %rsp, %rdi\n"
                 "cld\n"
                 "call isr_handler\n"
                 "popq %rdi\n"
                 "popq %rsi\n"
                 "popq %rbp\n"
                 "addq $8, %rsp\n"
                 "popq %rbx\n"
                 "popq %rdx\n"
                 "popq %rcx\n"
                 "popq %rax\n"
                 "popq %r8\n"
                 "popq %r9\n"
                 "popq %r10\n"
                 "popq %r11\n"
                 "popq %r12\n"
                 "popq %r13\n"
                 "popq %r14\n"
                 "popq %r15\n"
                 /* account for pushed isr_number and exception_code */
                 "addq $16, %rsp\n"
                 "iretq\n");
}

// clang-format off
#define __define_exception_no_ec(_number)                                      \
    static __noinline __naked void exception##_number(void) {                  \
        asm volatile("pushq $0x00\n"                                                    \
            "pushq $" STRINGIFY(_number) "\n"                                  \
            "jmp isr_common_stub\n");                                          \
    }

#define __define_exception_with_ec(_number)                                    \
    static __noinline __naked void exception##_number(void) {                  \
        asm volatile("pushq $" STRINGIFY(_number) "\n"                                  \
            "jmp isr_common_stub\n");                                          \
    }

__define_exception_no_ec(0)
__define_exception_no_ec(1)
__define_exception_no_ec(2)
__define_exception_no_ec(3)
__define_exception_no_ec(4)
__define_exception_no_ec(5)
__define_exception_no_ec(6)
__define_exception_no_ec(7)
__define_exception_with_ec(8)
__define_exception_no_ec(9)
__define_exception_with_ec(10)
__define_exception_with_ec(11)
__define_exception_with_ec(12)
__define_exception_with_ec(13)
__define_exception_with_ec(14)
__define_exception_no_ec(15)
__define_exception_no_ec(16)
__define_exception_no_ec(17)
__define_exception_no_ec(18)
__define_exception_no_ec(19)
__define_exception_no_ec(20)
__define_exception_no_ec(21)
__define_exception_no_ec(22)
__define_exception_no_ec(23)
__define_exception_no_ec(24)
__define_exception_no_ec(25)
__define_exception_no_ec(26)
__define_exception_no_ec(27)
__define_exception_no_ec(28)
__define_exception_no_ec(29)
__define_exception_no_ec(30)
__define_exception_no_ec(31)

#define _ISR(_number)                                                          \
    static __noinline __naked void isr##_number(void) {                        \
        asm volatile("pushq $0x00\n"                                                    \
            "pushq $" STRINGIFY(_number) "\n"                                  \
            "jmp isr_common_stub\n");                                          \
    }

ENUMERATE_ISRS

#undef _ISR

void init_idt(void) {
    set_idt_entry(0, (u64)exception0);
    set_idt_entry(1, (u64)exception1);
    set_idt_entry(2, (u64)exception2);
    set_idt_entry(3, (u64)exception3);
    set_idt_entry(4, (u64)exception4);
    set_idt_entry(5, (u64)exception5);
    set_idt_entry(6, (u64)exception6);
    set_idt_entry(7, (u64)exception7);
    set_idt_entry(8, (u64)exception8);
    set_idt_entry(9, (u64)exception9);
    set_idt_entry(10, (u64)exception10);
    set_idt_entry(11, (u64)exception11);
    set_idt_entry(12, (u64)exception12);
    set_idt_entry(13, (u64)exception13);
    set_idt_entry(14, (u64)exception14);
    set_idt_entry(15, (u64)exception15);
    set_idt_entry(16, (u64)exception16);
    set_idt_entry(17, (u64)exception17);
    set_idt_entry(18, (u64)exception18);
    set_idt_entry(19, (u64)exception19);
    set_idt_entry(20, (u64)exception20);
    set_idt_entry(21, (u64)exception21);
    set_idt_entry(22, (u64)exception22);
    set_idt_entry(23, (u64)exception23);
    set_idt_entry(24, (u64)exception24);
    set_idt_entry(25, (u64)exception25);
    set_idt_entry(26, (u64)exception26);
    set_idt_entry(27, (u64)exception27);
    set_idt_entry(28, (u64)exception28);
    set_idt_entry(29, (u64)exception29);
    set_idt_entry(30, (u64)exception30);
    set_idt_entry(31, (u64)exception31);

#define _ISR(_number) set_idt_entry(_number, (u64)isr##_number);
    ENUMERATE_ISRS
#undef _ISR

    // Remap the PIC
    port_out8(0x20, 0x11);
    port_out8(0xA0, 0x11);
    port_out8(0x21, 0x20);
    port_out8(0xA1, 0x28);
    port_out8(0x21, 0x04);
    port_out8(0xA1, 0x02);
    port_out8(0x21, 0x01);
    port_out8(0xA1, 0x01);
    port_out8(0x21, 0x0);
    port_out8(0xA1, 0x0);


    load_idt();
    sti();
}
