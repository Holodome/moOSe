//
// RTC (real-time clock)
//
#pragma once

#include <types.h>

struct ktm;

size_t get_systemtick(void);
size_t get_seconds(void);

void init_rtc(void);
void rtc_read_tm(struct ktm *tm);
