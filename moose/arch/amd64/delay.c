#include <moose/arch/amd64/asm.h>
#include <moose/arch/delay.h>

void delay_us(u32 us) {
    while (us--)
        (void)port_in8(0x80);
}
