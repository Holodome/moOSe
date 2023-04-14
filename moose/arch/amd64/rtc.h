//
// RTC (real-time clock)
//
#pragma once

#include <types.h>

struct ktm;

void init_rtc(void);
void rtc_read_tm(struct ktm *tm);
