#pragma once

#include "types.h"

u8 port_u8_in(u16 port);
u16 port_u16_in(u16 port);
void port_u8_out(u16 port, u8 data);
void port_u16_out(u16 port, u16 data);
