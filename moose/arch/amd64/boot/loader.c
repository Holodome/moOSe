#include <../arch/amd64/ata.c>
#include <../disk.c>
#include <../fs/fat.c>

int load_kernel(void) {
    /* return 0; */
    return ata_pio_read((void *)0x100000, 18, 1024);
}
