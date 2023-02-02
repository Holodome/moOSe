#!/usr/bin/env python3

import struct
import sys

boot_name, stage2_name, kernel_name, out_name = sys.argv[1:]

boot = open(boot_name, "rb").read()
stage2 = open(stage2_name, "rb").read()
kernel = open(kernel_name, "rb").read()

stage2_size = (len(stage2) + 511) // 512
kernel_offset = 1 + stage2_size
kernel_size = (len(kernel) + 511) // 512


def make_part_entry(offset, size):
    arr = bytearray([0] * 16)
    arr[0] = 0x80
    arr[8:12] = struct.pack("<I", offset)
    arr[12:16] = struct.pack("<I", size)
    print(arr)
    return arr


with open(out_name, "wb+") as f:
    f.write(boot)
    f.seek(0x01be)
    f.write(make_part_entry(kernel_offset, kernel_size))
    f.seek(512)
    f.write(stage2)
    f.seek(kernel_offset * 512)
    f.write(kernel)
    f.seek(0x01b0)
    f.write(struct.pack("<H", stage2_size))
