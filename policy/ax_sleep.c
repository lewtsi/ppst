#include "ax_wakeup.h"

#define FX_SLEEP_INTERVAL	10		//<! �������ʱ����
#define FX_INTERVAL_WAKEUP_TIME	1	//<! ������ѹ���ʱ��

static uint8_t 
static uint32_t fx_rtc_now, fx_interval_time, fx_fixed_time;



/// ��ȡ��ǰʱ����Ϣ
_fx uint32_t fx_generate_interval_time_info(void)
{
	return RTC_GetCounter();
}

/// ��ȡ������Ϣ
_fx uint32_t fx_generate_fixed_time_info(void)
{
	return 
}

/// ���㻽�ѣ������Ϣ��ȡ
_fx opt_result_t fx_fixed_time_enable_prehandle(void)
{

}

/// ������������ϢԤ����
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

/// ���û���ʱ�䣬����������
void ax_sleep_RTC_alarm_STANDBY_entrance(void)
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(fx_generate_alarm_time);
	RTC_WaitForLastTask();
	PWR_EnterSTANDBYMode();
}

/// �����û���ʱ�䣬������������
void  ax_sleep_RTC_alarm_setup(void)
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(fx_generate_alarm_time);
	RTC_WaitForLastTask();
}

/// �������ߣ�������������
/// ִ��ǰ��ȷ�ϻ���ʱ��������ȷ
void ax_sleep_RTC_STANDBY_entrance(void)
{
	PWR_EnterSTANDBYMode();
}


