#include <arch/amd64/rtc.h>
#include <sched/process.h>
#include <time.h>
#include <arch/cpu.h>
#include <arch/amd64/asm.h>
#include <arch/amd64/idt.h>

#define RATE 8
#define fREQUENCY (32768 >> (RATE - 1))

#define REG_SECS 0x00
#define REG_MINS 0x02
#define REG_HOURS 0x04
#define REG_DAY 0x07
#define REG_MON 0x08
#define REG_YEAR 0x09
#define REG_STATA 0x0a
#define REG_STATB 0x0b

static volatile u64 jiffies;

static u8 cmos_read(u8 idx) {
    port_out8(0x70, idx);
    return port_in8(0x71);
}

static void cmos_write(u8 idx, u8 data) {
    port_out8(0x70, idx);
    port_out8(0x71, data);
}

static void timer_interrupt(struct registers_state *) {
    ++jiffies;
    (void)cmos_read(0x8c);

    get_current()->needs_resched = 1;
}

void init_rtc(void) {
    register_isr(8, timer_interrupt);

    u8 prev;
    u8 rate = RATE;

    irq_disable();
    port_out8(0x70, 0x8a);
    port_out8(0x71, 0x20);

    prev = cmos_read(0x8b);
    cmos_write(0x8b, prev | 0x40);
    // register frequency
    prev = cmos_read(0x8a);
    cmos_write(0x8a, (prev & 0xf0) | rate);
    irq_enable();
}

u32 get_jiffies(void) {
    return (u32)jiffies;
}
u64 get_jiffies64(void) {
    return jiffies;
}
u64 jiffies64_to_msecs(u64 jiffies) {
    return jiffies * 1000 / fREQUENCY;
}
u64 msecs_to_jiffies64(u64 msecs) {
    return msecs * 1000 * fREQUENCY;
}
u32 jiffies_to_msecs(u32 jiffies) {
    return jiffies * 1000 / fREQUENCY;
}
u32 msecs_to_jiffies(u32 msecs) {
    return msecs * 1000 * fREQUENCY;
}

static int is_update_in_progress(void) {
    return (cmos_read(REG_STATA) & 0x80) != 0;
}

static u8 decode_bcd_u8(u8 bcd) {
    return (bcd & 0x0f) + ((bcd >> 4) * 10);
}

void rtc_read_tm(struct ktm *tm) {
    while (is_update_in_progress())
        spinloop_hint();

    int sec, min, hour, day, mon, year;
    sec = cmos_read(REG_SECS);
    min = cmos_read(REG_MINS);
    hour = cmos_read(REG_HOURS);
    day = cmos_read(REG_DAY);
    mon = cmos_read(REG_MON);
    year = cmos_read(REG_YEAR);

    int osec, omin, ohour, oday, omon, oyear;
    do {
        osec = sec;
        omin = min;
        ohour = hour;
        oday = day;
        omon = mon;
        oyear = year;

        while (is_update_in_progress())
            spinloop_hint();

        sec = cmos_read(REG_SECS);
        min = cmos_read(REG_MINS);
        hour = cmos_read(REG_HOURS);
        day = cmos_read(REG_DAY);
        mon = cmos_read(REG_MON);
        year = cmos_read(REG_YEAR);
    } while (osec != sec || omin != min || ohour != hour || oday != day ||
             omon != mon || oyear != year);

    int regb = cmos_read(REG_STATB);
    if (!(regb & 0x04)) {
        sec = decode_bcd_u8(sec);
        min = decode_bcd_u8(min);
        hour = decode_bcd_u8(hour & 0x7f);
        day = decode_bcd_u8(day);
        mon = decode_bcd_u8(mon);
        year = decode_bcd_u8(year);
    }

    if (!(regb & 0x02) && (hour & 0x80))
        hour = ((hour & 0x7f) + 12) % 24;

    tm->tm_sec = sec;
    tm->tm_min = min;
    tm->tm_hour = hour;
    tm->tm_mday = day;
    tm->tm_mon = mon - 1;
    tm->tm_year = 100 + year;
}

time_t current_time(void) {
    struct ktm tm;
    rtc_read_tm(&tm);
    return ktm_to_time(&tm);
}

void current_time_tm(struct ktm *tm) {
    rtc_read_tm(tm);
}
