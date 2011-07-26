#include "ax_pwm.h"
#include "ax_stdlib.h"

#define TIM_REBOOT_TIME		((uint16_t)30)	// ms

static TIM_ICInitTypeDef PWMIN_ICInitStructure;
static uint16_t fx_pwm_duty_cycle[4];
static uint32_t fx_pwm_frequency[4];
static volatile uint8_t fx_pwm_input_counting = 0;
static volatile uint16_t fx_ic1value, fx_ic2value;

// PB0 - TIM3_CH3
void ax_pwm_output_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = 999;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 500;	// 50% PWM OUTPUT
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);
	TIM_Cmd(TIM3, ENABLE);
}

void ax_pwm_output_config(uint8_t percentage)
{
	uint16_t ic3_value = percentage * 10;
	TIM_SetCompare3(TIM3, ic3_value);
}

_fx void fx_pwm_input_tim_enable(TIM_TypeDef* TIMx, uint16_t TIM_CHNL)
{
	PWMIN_ICInitStructure.TIM_Channel = TIM_CHNL;
	TIM_PWMIConfig(TIMx, &PWMIN_ICInitStructure);

	if(TIM_CHNL == TIM_Channel_1)
		TIM_SelectInputTrigger(TIMx, TIM_TS_TI1FP1);
	else
		TIM_SelectInputTrigger(TIMx, TIM_TS_TI2FP2);
	TIM_SelectSlaveMode(TIMx, TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIMx, TIM_MasterSlaveMode_Enable);

	TIM_Cmd(TIMx, ENABLE);
	delay_ms(TIM_REBOOT_TIME);
	TIM_SetCounter(TIMx, 0);
	TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
	if(TIM_CHNL == TIM_Channel_1)
		TIM_ITConfig(TIMx, TIM_IT_CC1, ENABLE);
	else
		TIM_ITConfig(TIMx, TIM_IT_CC2, ENABLE);
}

// A0 A1 - TIM2-CH1 CH2 | B6 B7 - TIM4-CH1 CH2
void ax_pwm_input_multi_channel_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM4, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	//PWMIN_ICInitStructure.TIM_Channel = TIM_Channel_2;
	PWMIN_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	PWMIN_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	PWMIN_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	PWMIN_ICInitStructure.TIM_ICFilter = 0x0;
	//TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);
}

_fx void fx_pwm_input_tim_disable(TIM_TypeDef* TIMx, uint16_t TIM_IT)
{
	TIM_ITConfig(TIMx, TIM_IT, DISABLE);
	TIM_Cmd(TIMx, DISABLE);
}

_fx void fx_pwm_input_value_handle(uint16_t TIM_CHNL, uint8_t arr_pos)
{
	if((TIM_CHNL == TIM_Channel_1) && (fx_ic1value != 0)){
		fx_pwm_duty_cycle[arr_pos] = (fx_ic2value * 100) / fx_ic1value;
		fx_pwm_frequency[arr_pos] = 1000000 / fx_ic1value;
	}else if((TIM_CHNL == TIM_Channel_2) && (fx_ic2value != 0)){
		fx_pwm_duty_cycle[arr_pos] = (fx_ic1value * 100) / fx_ic2value;
		fx_pwm_frequency[arr_pos] = 1000000 / fx_ic2value;
	}else{
		fx_pwm_duty_cycle[arr_pos] = 0;
		fx_pwm_frequency[arr_pos] = 0;
	}
}

opt_result_t ax_pwm_input_get_info(uint16_t **duty_cycle, uint32_t **freq)
{
	uint8_t i;
	uint8_t opt_result = 0;

	fx_pwm_input_counting = 1;
	fx_pwm_input_tim_enable(TIM2, TIM_Channel_1);
	while(fx_pwm_input_counting);
	fx_pwm_input_tim_disable(TIM2, TIM_IT_CC1);
	fx_pwm_input_value_handle(TIM_Channel_1, 0);
	
	fx_pwm_input_counting = 1;
	fx_pwm_input_tim_enable(TIM2, TIM_Channel_2);
	while(fx_pwm_input_counting);
	fx_pwm_input_tim_disable(TIM2, TIM_IT_CC2);
	fx_pwm_input_value_handle(TIM_Channel_2, 1);
	
	fx_pwm_input_counting = 1;
	fx_pwm_input_tim_enable(TIM4, TIM_Channel_1);
	while(fx_pwm_input_counting);
	fx_pwm_input_tim_disable(TIM4, TIM_IT_CC1);
	fx_pwm_input_value_handle(TIM_Channel_1, 2);
	
	fx_pwm_input_counting = 1;
	fx_pwm_input_tim_enable(TIM4, TIM_Channel_2);
	while(fx_pwm_input_counting);
	fx_pwm_input_tim_disable(TIM4, TIM_IT_CC2);
	fx_pwm_input_value_handle(TIM_Channel_2, 3);

	for(i=0; i<4; i++){
		if(fx_pwm_duty_cycle[i] > 100) opt_result |= (1<<i);
		if(fx_pwm_frequency[i] > 10000000) opt_result |= (1<<(i+4));
	}
	*duty_cycle = fx_pwm_duty_cycle;
	*freq = fx_pwm_frequency;

	return (opt_result_t)opt_result;
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_CC1 | TIM_IT_CC2)){
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1 | TIM_IT_CC2);

		fx_ic1value = TIM_GetCapture1(TIM2) + 1;
		fx_ic2value = TIM_GetCapture2(TIM2) + 1;
		fx_pwm_input_counting = 0;
	}else if(TIM_GetITStatus(TIM2, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		fx_ic1value = 0;
		fx_ic2value = 0;
		fx_pwm_input_counting = 0;
	}
}

void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_CC1 | TIM_IT_CC2)){
		TIM_ClearITPendingBit(TIM4, TIM_IT_CC1 | TIM_IT_CC2);

		fx_ic1value = TIM_GetCapture1(TIM4) + 1;
		fx_ic2value = TIM_GetCapture2(TIM4) + 1;
		fx_pwm_input_counting = 0;
	}else if(TIM_GetITStatus(TIM4, TIM_IT_Update)){
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);

		fx_ic1value = 0;
		fx_ic2value = 0;
		fx_pwm_input_counting = 0;
	}
}



/*EOF*/

