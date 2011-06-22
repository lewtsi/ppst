#ifndef AX_RTC_H
#define AX_RTC_H

#include "ax_common.h"

opt_result_t ax_rtc_init(void);
void ax_rtc_disable(void);
void ax_rtc_enable(void);
uint8_t ax_rtc_get_sec_flag(void);
void ax_rtc_set_sec_flag(uint8_t rtcflag);
uint32_t ax_rtc_get_now_time(void);
void ax_rtc_set_now_time(uint32_t rtctime);
uint16_t ax_rtc_get_day_counter(void);
void ax_rtc_set_day_counter(uint16_t daycnt);

#endif

