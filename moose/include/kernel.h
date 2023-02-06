#pragma once

#define KERNEL_VIRTUAL_BASE 0xffff880000000000
#define KERNEL_PHYSICAL_BASE 0x100000
#define FIXUP_POINTER(_mem) ((_mem) + KERNEL_VIRTUAL_BASE)
