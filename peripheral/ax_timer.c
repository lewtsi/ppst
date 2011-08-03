
#include "ax_timer.h"

#define FX_TIMA			TIM6
#define FX_TIMA_RCC		RCC_APB1Periph_TIM6
#define FX_TIMA_IRQ		TIM6_IRQn

#define FX_TIMB			TIM7
#define FX_TIMB_RCC		RCC_APB1Periph_TIM7
#define FX_TIMB_IRQ		TIM7_IRQn

#define FX_TIMUSART		TIM5
#define FX_TIMUSART_RCC	RCC_APB1Periph_TIM5
#define FX_TIMUSART_IRQ	TIM5_IRQn

static uint8_t fx_timA_running_flag, fx_timB_running_flag;
static volatile uint8_t fx_timA_OV_flag, fx_timB_OV_flag;
static volatile uint8_t fx_timUsart_OV_flag;

void ax_timA_init(ax_tim_initValue_t initValue)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(FX_TIMA_RCC, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = FX_TIMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = (uint16_t)initValue;
	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024->7200
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;   
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(FX_TIMA, &TIM_TimeBaseStructure);
	FX_TIMB->CNT = 0;
	TIM_ClearFlag(FX_TIMA, TIM_FLAG_Update);
	TIM_ITConfig(FX_TIMA, TIM_IT_Update, ENABLE);
	TIM_Cmd(FX_TIMA, ENABLE);

	fx_timA_running_flag = FLAG_FALSE;
	fx_timA_OV_flag = FLAG_FALSE;
}

void ax_timA_enable(ax_tim_initValue_t initValue)
{
	FX_TIMA->ARR = (uint16_t)initValue;
	FX_TIMA->CNT = 0;
	fx_timA_running_flag = FLAG_TRUE;
	fx_timA_OV_flag = FLAG_FALSE;
	TIM_Cmd(FX_TIMA, ENABLE);
}

void ax_timA_continue(void)
{
	fx_timA_running_flag = FLAG_TRUE;
	TIM_Cmd(FX_TIMA, ENABLE);
}

void ax_timA_disable(void)
{
	fx_timA_running_flag = FLAG_FALSE;
	TIM_Cmd(FX_TIMA, DISABLE);
}

uint8_t ax_timA_get_ov_flag(void)
{
	return fx_timA_OV_flag;
}

void ax_timA_set_ov_flag(uint8_t tflag)
{
	fx_timA_OV_flag = tflag;
}

uint8_t ax_timA_get_status(void)
{
	return (fx_timA_running_flag);
}

void ax_timB_init(ax_tim_initValue_t initValue)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(FX_TIMB_RCC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = FX_TIMB_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = (uint16_t)initValue;
	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024->7200
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(FX_TIMB, &TIM_TimeBaseStructure);
	FX_TIMB->CNT = 0;
	TIM_ClearFlag(FX_TIMB, TIM_FLAG_Update);
	TIM_ITConfig(FX_TIMB, TIM_IT_Update, ENABLE);
	TIM_Cmd(FX_TIMB, ENABLE);

	fx_timB_running_flag = FLAG_FALSE;
	fx_timB_OV_flag = FLAG_FALSE;
}

void ax_timB_enable(ax_tim_initValue_t initValue)
{
	FX_TIMB->ARR = (uint16_t)initValue;
	FX_TIMB->CNT = 0;
	fx_timB_OV_flag = FLAG_FALSE;
	TIM_Cmd(FX_TIMB, ENABLE);
	fx_timB_running_flag = FLAG_TRUE;
}

void ax_timB_continue(void)
{
	fx_timB_running_flag = FLAG_TRUE;
	TIM_Cmd(FX_TIMB, ENABLE);
}

void ax_timB_disable(void)
{
	fx_timB_running_flag = FLAG_FALSE;
	TIM_Cmd(FX_TIMB, DISABLE);
}

uint8_t ax_timB_get_ov_flag(void)
{
	return fx_timB_OV_flag;
}

void ax_timB_set_ov_flag(uint8_t tflag)
{
	fx_timB_OV_flag = tflag;
}

uint8_t ax_timB_get_status(void)
{
	return (fx_timB_running_flag);
}

void ax_timUsart_init(ax_tim_initValue_t initValue)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(FX_TIMUSART_RCC, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = FX_TIMUSART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = (uint16_t)initValue;
	TIM_TimeBaseStructure.TIM_Prescaler = 9999; // 7.3728M/1024->7200
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;   
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(FX_TIMUSART, &TIM_TimeBaseStructure);
	TIM_ClearFlag(FX_TIMUSART, TIM_FLAG_Update);
	TIM_ITConfig(FX_TIMUSART, TIM_IT_Update, ENABLE);
	TIM_Cmd(FX_TIMUSART, DISABLE);	//ENABLE

	fx_timUsart_OV_flag = FLAG_FALSE;
}

void ax_timUsart_enable(void)
{
	TIM_Cmd(FX_TIMUSART, ENABLE);
}

void ax_timUsart_disable(void)
{
	TIM_Cmd(FX_TIMUSART, DISABLE);
}

void ax_timUsart_counter_clear(void)
{
	TIM_SetCounter(FX_TIMUSART, 0);
}

uint8_t ax_timUsart_get_OV_flag(void)
{
	return (fx_timUsart_OV_flag);
}

void ax_timUsart_set_OV_flag(uint8_t tflag)
{
	fx_timUsart_OV_flag = tflag;
}

void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(FX_TIMA, TIM_IT_Update)!= RESET){
		TIM_ClearITPendingBit(FX_TIMA, TIM_FLAG_Update);
		fx_timA_OV_flag = FLAG_TRUE;
	}
}

void TIM7_IRQHandler(void)
{
	if(TIM_GetITStatus(FX_TIMB, TIM_IT_Update)!= RESET){
		TIM_ClearITPendingBit(FX_TIMB, TIM_FLAG_Update);
		fx_timB_OV_flag = FLAG_TRUE;
	}
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(FX_TIMUSART, TIM_IT_Update)!= RESET){
		TIM_ClearITPendingBit(FX_TIMUSART, TIM_FLAG_Update);
		fx_timUsart_OV_flag = FLAG_TRUE;	// 初值将在USART的中断函数中赋值
		TIM_Cmd(FX_TIMUSART, DISABLE);
	}
}


/* EOF */



