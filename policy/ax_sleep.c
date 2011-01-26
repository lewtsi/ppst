#include "ax_wakeup.h"

#define FX_SLEEP_INTERVAL	10		//<! 间隔唤醒时间间隔
#define FX_INTERVAL_WAKEUP_TIME	1	//<! 间隔唤醒工作时间

static uint8_t 
static uint32_t fx_rtc_now, fx_interval_time, fx_fixed_time;



/// 获取当前时间信息
_fx uint32_t fx_generate_interval_time_info(void)
{
	return RTC_GetCounter();
}

/// 获取定点信息
_fx uint32_t fx_generate_fixed_time_info(void)
{
	return 
}

/// 定点唤醒，相关信息提取
_fx opt_result_t fx_fixed_time_enable_prehandle(void)
{

}

/// 间隔唤醒相关信息预处理
_fx opt_result_t fx_interval_time_enable_prehandle(void)
{
	
}

_fx uint32_t fx_generate_alarm_time(void)
{
	fx_rtc_now = RTC_GetCounter();
	fx_interval_time = fx_generate_interval_time_info();
	fx_fixed_time = fx_generate_fixed_time_info();

	if(fx_fixed_time <= fx_interval_time){
		fx_fixed_time_enable_prehandle();
		return fx_fixed_time;
	}else{
		fx_interval_time_enable_prehandle();
		return fx_interval_time;
	}
}

/// 设置唤醒时间，并进入休眠
void ax_sleep_RTC_alarm_STANDBY_entrance(void)
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(fx_generate_alarm_time);
	RTC_WaitForLastTask();
	PWR_EnterSTANDBYMode();
}

/// 仅设置唤醒时间，而不进入休眠
void  ax_sleep_RTC_alarm_setup(void)
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(fx_generate_alarm_time);
	RTC_WaitForLastTask();
}

/// 进入休眠，不做其他操作
/// 执行前已确认唤醒时间设置正确
void ax_sleep_RTC_STANDBY_entrance(void)
{
	PWR_EnterSTANDBYMode();
}


