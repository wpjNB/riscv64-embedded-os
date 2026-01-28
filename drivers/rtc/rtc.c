#include "rtc.h"

#define RTC_BASE 0x101000UL

void rtc_init(void) {
    /* RTC initialization */
}

uint64_t rtc_get_time(void) {
    /* Read RTC time register */
    volatile uint32_t *rtc = (volatile uint32_t*)RTC_BASE;
    return *rtc;
}
