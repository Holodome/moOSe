#!/usr/bin/env python3

import struct
import sys
import os
import subprocess

boot_name, stage2_name, kernel_name, out_name = sys.argv[1:]

boot = open(boot_name, "rb").read()
stage2 = open(stage2_name, "rb").read()
kernel = open(kernel_name, "rb").read()

stage2_size = (len(stage2) + 511) // 512
kernel_offset = 1 + stage2_size
kernel_size = (len(kernel) + 511) // 512
ext2_offset = kernel_offset + kernel_size
ext2_size = (1 << 20) // 512


def gen_ext2(path, size):
    with open(path, "wb"):
        pass
    os.truncate(path, size)
    with open(os.devnull, 'wb') as devnull:
        subprocess.check_call(["mkfs.ext2", path], stdout=devnull,
                              stderr=subprocess.STDOUT)


def make_part_entry(offset, size):
    arr = bytearray([0] * 16)
    arr[0] = 0x80
    arr[8:12] = struct.pack("<I", offset)
    arr[12:16] = struct.pack("<I", size)
    return arr


gen_ext2("/tmp/ext2-moose", ext2_size * 512)

with open(out_name, "wb+") as f:
    f.write(boot)
    f.seek(0x01be)
    f.write(make_part_entry(kernel_offset, kernel_size))
    #f.seek(16, os.SEEK_CUR)
    f.write(make_part_entry(ext2_offset, ext2_size))
    f.seek(512)
    f.write(stage2)
    f.seek(kernel_offset * 512)
    f.write(kernel)
    f.seek(0x01b0)
    f.write(struct.pack("<H", stage2_size))
    f.seek(ext2_offset * 512)
    f.write(open("/tmp/ext2-moose", "rb").read())
