/** 
	��Ҫ�ṩ�Ĺ���:
	1��1s��ʱ���ܣ���ѯflag��־����
	2���ṩʱ�ӹ��ܣ���ȡ��ǰDAY_CNT & RTC_CNT
		Ϊ�˸��û����������ԣ����ﲢû����UTCʱ�䷽ʽ��
		�û����������������ں�RTC�Ŀ�ʼʱ��
		ע:65535�� = 179��200��

 **/

#include "ax_rtc.h"

static volatile uint8_t fx_rtc_flag;
static uint8_t fx_rtc_flag_mask;
static volatile uint16_t fx_rtc_day_counter;

opt_result_t ax_rtc_init(void)
{
	opt_result_t rtc_status;
	NVIC_InitTypeDef NVIC_InitStructure;//����NVIC��ʼ���ṹ�����

	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//����PWR��BKP��ʱ�ӣ�from APB1��
	// ���������ʹ��RTC��������Ҳ���뿪��ʱ�Ӳ����� !!!
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);	//�������
	if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5){
	    BKP_DeInit();	//���ݼĴ���ģ�鸴λ

	    RCC_LSEConfig(RCC_LSE_ON);	//�ⲿ32.768K
	    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	// �ȴ��ȶ�
	    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);	// RTCʱ��Դ���ó�LSE
	    
	    RCC_RTCCLKCmd(ENABLE);	//RTC����
	    //��������Ҫ�ȴ�APB1ʱ����RTCʱ��ͬ�������ܶ�д�Ĵ���
	    RTC_WaitForSynchro();
	    //��д�Ĵ���ǰ��Ҫȷ����һ�������Ѿ�����
	    RTC_WaitForLastTask();

	    //����RTC��Ƶ����ʹRTCʱ��Ϊ1Hz
	    //RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
	    RTC_SetPrescaler(32767);
	    RTC_WaitForLastTask();	//�ȴ��Ĵ���д�����
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
		rtc_status = OPT_RESULT_A;
	 }else{
		// �鿴���θ�λ����
		if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET){
			//�����ϵ縴λ
		}else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET){
			//�����ⲿRST�ܽŸ�λ
        }
		//���RCC�и�λ��־
		RCC_ClearFlag();
		
		rtc_status = OPT_RESULT_B;
	}
    RTC_WaitForLastTask();
	
    RTC_ClearITPendingBit(RTC_IT_SEC);
    RTC_WaitForLastTask();
    RTC_ITConfig(RTC_IT_SEC, ENABLE);	//ʹ�����ж�
    RTC_WaitForLastTask();

	fx_rtc_flag = FLAG_FALSE;
	fx_rtc_flag_mask = 0xFF;	// Ĭ�������û���ȡ
	fx_rtc_day_counter = 0;
	
	return rtc_status;
}

// ����RTC����ʱ��ʹ�ã�disable���ı��û���ȡֵ
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
		RTC_ClearITPendingBit(RTC_IT_SEC);//�����������RTC���ж�
		fx_rtc_flag = FLAG_TRUE;
		RTC_WaitForLastTask();//�ȴ�д�������
		//��RTCʱ��Ϊ24:00:00ʱ��λ������ 
		if(RTC_GetCounter() == 0x00015180){
			RTC_SetCounter(0x0); //RTC����������
			fx_rtc_day_counter ++;
			RTC_WaitForLastTask();//�ȴ�д�������
		}
	}
}


