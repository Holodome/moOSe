#include <arch/amd64/asm.h>
#include <arch/delay.h>

void delay_us(u32 us) {
    while (us--)
        (void)port_in8(0x80);
}
