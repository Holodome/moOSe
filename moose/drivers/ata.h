#pragma once

#include <types.h>

int ata_read_block(size_t idx, void *buf);
int ata_write_block(size_t idx, const void *buf);
