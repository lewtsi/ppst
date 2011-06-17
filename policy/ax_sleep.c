#include "ax_wakeup.h"

#define FX_SLEEP_INTERVAL	10		//<! 间隔唤醒时间间隔
#define FX_INTERVAL_WAKEUP_TIME	1	//<! 间隔唤醒工作时间

/// 注意: 定点唤醒之后将BKP_DR中的数据置为TYPE_INTVL
/// 	定点休眠前将其置为TYPE_FIXED，其他地方无需写入操作
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
		fixed_cnt_now ++;	// 当前时刻的下一时刻
		ram_fixed_time = flash_read_buf[8];		// 定点时刻缓存头
		ram_fixed_time += fixed_cnt_now << 2;	// 指向下一定点时刻
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
	if(fixed_cnt_now < fixed_totle_cnt){	// 表示定点时刻依然有效
		if((interval_time + wakeup_time + 1) > fixed_time){    // fixed time
			wakeup_time = fixed_time;
			BKP_WriteBackupRegister(BKP_DR_WAKEUP_TYPE, FX_WAKEUP_TYPE_FIXED);
		}
	}
	
	return wakeup_time;	
}

/// 休眠功能启用的上电处理函数
/// 首先读取文件查看与flash内部的内容是否匹配，如必要则更新flash内容 ，
/// 同时将第一个定点时间放入BKP
/// 如果没有定点时刻文件，则在flash中写入相关信息
/// 最多读取24个信息
void ax_sleep_function_prehandle(void)
{
	
}

/// 设置唤醒时间，并进入休眠
void ax_sleep_alarm_STANDBY_entrance(void)
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(fx_generate_alarm_time);
	RTC_WaitForLastTask();
	PWR_EnterSTANDBYMode();
}

/// 仅设置唤醒时间，而不进入休眠
void  ax_sleep_setup_alarm_time(void)
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


