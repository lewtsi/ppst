
#include "ax_timer.h"

static volatile uint8_t fx_timerA_flag;
static volatile uint16_t fx_timerA_counter;

void ax_timerA_init(tcnt16_t TCN_vlaue)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(TIMERA_RCC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIMERA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = (uint16_t)TCN_vlaue;
	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024=7200<=>72M/10000
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;   
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //选择向上计数
	TIM_TimeBaseInit(TIMERA, &TIM_TimeBaseStructure);
	TIM_ClearFlag(TIMERA, TIM_FLAG_Update);	// clear interrupt flag
	TIM_ITConfig(TIMERA, TIM_IT_Update, ENABLE);	// timer interrupt enable
	TIM_Cmd(TIMERA, ENABLE); // timer enable

	fx_timerA_flag = 0;
	fx_timerA_counter = 0;
}


void ax_timerA_disable(void)
{
	TIM_Cmd(TIMERA, DISABLE);
}

void ax_timerA_enable(void)
{
	TIM_Cmd(TIMERA, ENABLE);
}

uint8_t ax_get_timerA_flag(void)
{
	return fx_timerA_flag;
}

void ax_set_timerA_flag(uint8_t tflag)
{
	fx_timerA_flag = tflag;
}

uint16_t ax_get_timerA_counter(void)
{
	return fx_timerA_counter;
}

void ax_set_timerA_counter(uint16_t cnt)
{
	fx_timerA_counter = cnt;
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIMERA, TIM_IT_Update)!= RESET){
		TIM_ClearITPendingBit(TIMERA, TIM_FLAG_Update);
		fx_timerA_flag = 0x01;
		fx_timerA_counter ++;
	}
}




