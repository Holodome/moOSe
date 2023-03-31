#include <time.h>

#include <assert.h>
#include <kstdio.h>

time_t ktm_to_time(const struct ktm *tm) {
    time_t days = days_since_epoch(tm->tm_year + 1900, tm->tm_mon, tm->tm_mday);
    return ((((days * 24) + tm->tm_hour) * 60) + tm->tm_min) * 60 + tm->tm_sec;
}

static int leap_years_before(int year) {
    --year;
    return (year / 4) - (year / 100) - (year / 400);
}

static int leap_years_between(int a, int b) {
    return leap_years_before(b) - leap_years_before(a + 1);
}

static int is_year_leap(int year) {
    return (year % 400) == 0 || ((year % 100) != 0 && (year % 4 == 0));
}

time_t days_since_epoch(int year, int month, int day) {
    expects(month < 12);
    if (year < 1970)
        return 0;

    int count_of_leap_years = leap_years_between(1970, year);
    time_t days = 365 * (year - 1970) + count_of_leap_years;

    static const int days_in_months[] = {31, 28, 31, 30, 31, 30,
                                         31, 31, 30, 31, 30, 31};
    if (is_year_leap(year) && month >= 1)
        ++days; // feb
    for (int i = 0; i < month; ++i)
        days += days_in_months[i];
    days += day;

    return days;
}

void print_tm(const struct ktm *tm) {
    kprintf("sec=%d min=%d hour=%d day=%d mon=%d year=%d\n", tm->tm_sec,
            tm->tm_min, tm->tm_hour, tm->tm_mday, tm->tm_mon,
            1900 + tm->tm_year);
}
