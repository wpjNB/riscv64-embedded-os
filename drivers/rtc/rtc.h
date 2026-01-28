#ifndef _RTC_H
#define _RTC_H

#include "../types.h"

/* RTC initialization */
void rtc_init(void);

/* Get current time */
uint64_t rtc_get_time(void);

#endif /* _RTC_H */
