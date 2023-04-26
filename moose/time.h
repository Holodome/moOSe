#pragma once

#include <moose/types.h>

// Stripped down version of 'struct tm' from libc
struct ktm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
};

struct ktimespec {
    time_t tv_sec;
    long tv_nsec;
};

time_t ktm_to_time(const struct ktm *tm);
time_t days_since_epoch(int year, int month, int day);
time_t current_time(void);
void current_time_tm(struct ktm *tm);

void print_tm(const struct ktm *tm);
