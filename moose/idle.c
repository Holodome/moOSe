#include <arch/cpu.h>
#include <arch/amd64/rtc.h>
#include <drivers/disk.h>

void idle_task(void) {
    init_rtc();
    init_disk();

    for (;;) {
        wait_for_int();
    }
}
