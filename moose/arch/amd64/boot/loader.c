#include "../../../drivers/ata.c"

#include <mbr.h>
#include <param.h>

extern void print(const char *s);

int load_kernel(void) {
    char buffer[512];
    ata_read_block(0, buffer);
    struct mbr_partition partition;
    __builtin_memcpy(&partition, buffer + MBR_PARTITION_OFFSET,
                     sizeof(partition));
    size_t kernel_size = partition.size;
    size_t kernel_offset = partition.addr;

    uintptr_t cursor = KERNEL_PHYSICAL_BASE;
    for (; kernel_size--; cursor += sizeof(buffer))
        ata_read_block(kernel_offset++, (void *)cursor);

    return 0;
}
