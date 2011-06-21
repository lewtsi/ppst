#include "ax_pwm.h"

#define FX_PWM_OUT_TIM	TIM2
#define FX_PWM_OUT_PORT	GPIOA
#define FX_PWM_OUT_PIN1	GPIO_Pin_2
#define FX_PWM_OUT_PIN2	GPIO_Pin_3
#define fx_pwm_out_rcc()	\
	do{	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); \
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);}while(0)

#define FX_PWM_IN_TIM	TIM2
#define FX_PWM_IN_IRQ	TIM2_IRQn
#define FX_PWM_IN_PORT	GPIOA
#define FX_PWM_IN_PIN1	GPIO_Pin_1	// PA1->TIM2_CH2
#define fx_pwm_in_rcc()	\
	do{	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); \
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);}while(0)

static uint16_t fx_CCR3_Val = 500;
static uint16_t fx_CCR4_Val = 300;
static uint16_t fx_duty_cycle;
static uint32_t fx_frequency;


/* -----------------------------------------------------------------------
TIM2 Configuration: generate 2 PWM signals with 2 different duty cycles:
TIM2CLK = 36 MHz, Prescaler = 0x0, TIM2 counter clock = 36 MHz
TIM2 ARR Register = 999 => TIM2 Frequency = TIM2 counter clock/(ARR + 1)
TIM2 Frequency = 36 KHz.
TIM2 Channel3 duty cycle = (TIM2_CCR3/ TIM2_ARR)* 100 = 50%
TIM2 Channel4 duty cycle = (TIM2_CCR4/ TIM2_ARR)* 100 = 30%
----------------------------------------------------------------------- */
void ax_pwm_output_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	fx_pwm_out_rcc();

	GPIO_InitStructure.GPIO_Pin =  FX_PWM_OUT_PIN1 | FX_PWM_OUT_PIN2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(FX_PWM_OUT_PORT, &GPIO_InitStructure);

	TIM_TimeBaseStructure.TIM_Period = 999;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(FX_PWM_OUT_TIM, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = fx_CCR3_Val;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC3Init(FX_PWM_OUT_TIM, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(FX_PWM_OUT_TIM, TIM_OCPreload_Enable);

	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = fx_CCR4_Val;
	TIM_OC4Init(FX_PWM_OUT_TIM, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(FX_PWM_OUT_TIM, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(FX_PWM_OUT_TIM, ENABLE);
	TIM_Cmd(FX_PWM_OUT_TIM, ENABLE);
}

/* TIM2 configuration: PWM Input mode ------------------------
 The external signal is connected to TIM2 CH2 pin (PA.01), 
 The Rising edge is used as active edge,
 The TIM2 CCR2 is used to compute the frequency value 
 The TIM2 CCR1 is used to compute the duty cycle value
------------------------------------------------------------ */
void ax_pwm_input_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_ICInitTypeDef  TIM_ICInitStructure;
	
	fx_pwm_in_rcc();
	
	GPIO_InitStructure.GPIO_Pin = FX_PWM_IN_PIN1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(FX_PWM_IN_PORT, &GPIO_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = FX_PWM_IN_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_PWMIConfig(FX_PWM_IN_TIM, &TIM_ICInitStructure);

	/* Select the TIM2 Input Trigger: TI2FP2 */
	TIM_SelectInputTrigger(FX_PWM_IN_TIM, TIM_TS_TI2FP2);

	/* Select the slave Mode: Reset Mode */
	TIM_SelectSlaveMode(FX_PWM_IN_TIM, TIM_SlaveMode_Reset);

	/* Enable the Master/Slave Mode */
	TIM_SelectMasterSlaveMode(FX_PWM_IN_TIM, TIM_MasterSlaveMode_Enable);

	/* TIM enable counter */
	TIM_Cmd(FX_PWM_IN_TIM, ENABLE);

	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(FX_PWM_IN_TIM, TIM_IT_CC2, ENABLE);
}

opt_result_t ax_pwm_input_get_info(uint16_t *duty_cycle, uint32_t *freq)
{
	if(fx_duty_cycle > 100) return OPT_RESULT_FALSE;
	if(fx_frequency > 10000000) return OPT_RESULT_FALSE;
	*duty_cycle = fx_duty_cycle;
	*freq = fx_frequency;

	return OPT_RESULT_TRUE;
}

void TIM2_IRQHandler(void)
{
	uint16_t ic2value;
	
	TIM_ClearITPendingBit(FX_PWM_IN_TIM, TIM_IT_CC2);

	ic2value = TIM_GetCapture2(FX_PWM_IN_TIM);
	if (ic2value != 0){
		fx_duty_cycle = (TIM_GetCapture1(FX_PWM_IN_TIM) * 100) / ic2value;
		fx_frequency = 72000000 / ic2value;
	}else{
		fx_duty_cycle = 0;
		fx_frequency = 0;
	}
}



/*EOF*/

