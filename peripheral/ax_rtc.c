/** 
	主要提供的功能:
	1、1s定时功能，查询flag标志即可
	2、提供时钟功能，读取当前DAY_CNT & RTC_CNT
		为了给用户更大的灵活性，这里并没采用UTC时间方式，
		用户可以自由设置日期和RTC的开始时刻
		注:65535天 = 179年200天

 **/

#include "ax_rtc.h"

static volatile uint8_t fx_rtc_flag;
static uint8_t fx_rtc_flag_mask;
static volatile uint16_t fx_rtc_day_counter;

opt_result_t ax_rtc_init(void)
{
	opt_result_t rtc_status;
	NVIC_InitTypeDef NVIC_InitStructure;//定义NVIC初始化结构体变量

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//启用PWR和BKP的时钟（from APB1）
	// 重启后如果使用RTC唤醒休眠也必须开启时钟并解锁 !!!
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);	//后备域解锁
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5){
	    BKP_DeInit();	//备份寄存器模块复位

	    RCC_LSEConfig(RCC_LSE_ON);	//外部32.768K
	    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	// 等待稳定
	    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	// RTC时钟源配置成LSE
	    
	    RCC_RTCCLKCmd(ENABLE);	//RTC开启
	    //开启后需要等待APB1时钟与RTC时钟同步，才能读写寄存器
	    RTC_WaitForSynchro();
	    //读写寄存器前，要确定上一个操作已经结束
	    RTC_WaitForLastTask();

	    //设置RTC分频器，使RTC时钟为1Hz
	    //RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
	    RTC_SetPrescaler(32767);
	    RTC_WaitForLastTask();	//等待寄存器写入完成
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
		rtc_status = OPT_RESULT_A;
	 }else{
		// 查看本次复位类型
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET){
			//这是上电复位
		}else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET){
			//这是外部RST管脚复位
        }
		//清除RCC中复位标志
		RCC_ClearFlag();
		
		rtc_status = OPT_RESULT_B;
	}
    RTC_WaitForLastTask();
	
    RTC_ClearITPendingBit(RTC_IT_SEC);
    RTC_WaitForLastTask();
    RTC_ITConfig(RTC_IT_SEC, ENABLE);	//使能秒中断
    RTC_WaitForLastTask();

	fx_rtc_flag = FLAG_FALSE;
	fx_rtc_flag_mask = 0xFF;	// 默认允许用户读取
	fx_rtc_day_counter = 0;
	
	return rtc_status;
}

// 由于RTC兼做时钟使用，disable仅改变用户读取值
void ax_rtc_disable(void)
{
	fx_rtc_flag_mask = 0;
}

void ax_rtc_enable(void)
{
	fx_rtc_flag_mask = 0xFF;
}

uint8_t ax_get_rtc_flag(void)
{
	return (fx_rtc_flag & fx_rtc_flag_mask);
}

void ax_set_rtc_flag(uint8_t rtcflag)
{
	fx_rtc_flag = rtcflag;
}

uint32_t ax_get_rtc_time(void)
{
	return RTC_GetCounter();
}

void ax_set_rtc_time(uint32_t rtctime)
{
	RTC_SetCounter(rtctime);
}

uint16_t ax_get_day_counter(void)
{
	return fx_rtc_day_counter;
}

void ax_set_day_counter(uint16_t daycnt)
{
	fx_rtc_day_counter = daycnt;
}

void RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_SEC) != RESET){
		RTC_ClearITPendingBit(RTC_IT_SEC);//必须软件清零RTC秒中断
		fx_rtc_flag = FLAG_TRUE;
		RTC_WaitForLastTask();//等待写操作完成
		//当RTC时钟为24:00:00时复位计数器 
		if(RTC_GetCounter() == 0x00015180){
			RTC_SetCounter(0x0); //RTC计数器清零
			fx_rtc_day_counter ++;
			RTC_WaitForLastTask();//等待写操作完成
		}
	}
}


