#pragma once

struct ktm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int im_isdt;
};

struct ktimespec {
    time_t tv_sec;
    long tv_nsec;
};
