#include "ax_wakeup.h"

#define FX_SLEEP_INTERVAL	10		//<! �������ʱ����
#define FX_INTERVAL_WAKEUP_TIME	1	//<! ������ѹ���ʱ��

/// ע��: ���㻽��֮��BKP_DR�е�������ΪTYPE_INTVL
/// 	��������ǰ������ΪTYPE_FIXED�������ط�����д�����
#define FX_WAKEUP_TYPE_FIXED	(uint16_t)(0xAA55)
#define FX_WAKEUP_TYPE_INTVL	(uint16_t)(0x0000)

_fx uint32_t fx_generate_alarm_time(void)
{
	uint32_t rtc_now, interval_time, fixed_time, wakeup_time;
	uint8_t fixed_time_flag, fixed_cnt_now, fixed_totle_cnt, flash_read_len;
	char *flash_read_buf;
	uint32_t *ram_fixed_time;
	uint16_t fixed_run_time;

	rtc_now = RTC_GetCounter();
	interval_time = rtc_now + (FX_SLEEP_INTERVAL - FX_INTERVAL_WAKEUP_TIME);
	wakeup_time = interval_time;
	fixed_time_flag = 0;

	// fixed_totle_cnt = BKP_ReadBackupRegister(BKP_DR_FIXED_CNT_TOTLE);
	fixed_cnt_now = BKP_ReadBackupRegister(BKP_DR_FIXED_CNT_NOW);
	fixed_time = BKP_ReadBackupRegister(BKP_DR_FIXED_TIME_H);
	fixed_time <<= 16;
	fixed_time |= BKP_ReadBackupRegister(BKP_DR_FIXED_TIME_L);
	if(fixed_time < rtc_now){
		ax_flash_config_info_read(FLASH_SPACE_TYPE_FIXED_TIME,
									&flash_read_buf, &flash_read_len);
		fixed_cnt_now ++;	// ��ǰʱ�̵���һʱ��
		ram_fixed_time = flash_read_buf[8];		// ����ʱ�̻���ͷ
		ram_fixed_time += fixed_cnt_now << 2;	// ָ����һ����ʱ��
		fixed_totle_cnt = flash_read_buf[3];
		while(fixed_cnt_now < fixed_totle_cnt){
			if(*ram_fixed_time > fixed_cnt_now){
				break;
			}
			fixed_cnt_now ++;
			ram_fixed_time += 4;
		}
		BKP_WriteBackupRegister(BKP_DR_FIXED_CNT_NOW, (uint16_t)fixed_cnt_now);
		if(fixed_cnt_now < fixed_totle_cnt){
			fixed_time = *ram_fixed_time;
			BKP_WriteBackupRegister(BKP_DR_FIXED_TIME_H, 
												(uint16_t)(fixed_time >> 16));
			BKP_WriteBackupRegister(BKP_DR_FIXED_TIME_L, (uint16_t)(fixed_time));
			fixed_run_time = (uint16_t)*(ram_fixed_time + fixed_totle_cnt);
			BKP_WriteBackupRegister(BKP_DR_FIXED_RUN_TIME, fixed_run_time);
		}
	}
	if(fixed_cnt_now < fixed_totle_cnt){	// ��ʾ����ʱ����Ȼ��Ч
		if((interval_time + wakeup_time + 1) > fixed_time){    // fixed time
			wakeup_time = fixed_time;
			BKP_WriteBackupRegister(BKP_DR_WAKEUP_TYPE, FX_WAKEUP_TYPE_FIXED);
		}
	}
	
	return wakeup_time;	
}

/// ���߹������õ��ϵ紦����
/// ���ȶ�ȡ�ļ��鿴��flash�ڲ��������Ƿ�ƥ�䣬���Ҫ�����flash���� ��
/// ͬʱ����һ������ʱ�����BKP
/// ���û�ж���ʱ���ļ�������flash��д�������Ϣ
/// ����ȡ24����Ϣ
void ax_sleep_function_prehandle(void)
{
	
}

/// ���û���ʱ�䣬����������
void ax_sleep_alarm_STANDBY_entrance(void)
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(fx_generate_alarm_time);
	RTC_WaitForLastTask();
	PWR_EnterSTANDBYMode();
}

/// �����û���ʱ�䣬������������
void  ax_sleep_setup_alarm_time(void)
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


