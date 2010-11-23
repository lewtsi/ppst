#include "ax_system.h"

#include "ax_gpio.h"
#include "ax_usart.h"
#include "ax_systick.h"


_fx void fx_NVIC_Configuration(void)
{
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	
	NVIC_PriorityGroupConfig(NVIC_GLOBAL_PRIORITYGROUP);
}

_fx void fx_RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;

	RCC_DeInit();		// RCC system reset(for debug purpose)
	RCC_HSEConfig(RCC_HSE_ON);	// Enable HSE
	HSEStartUpStatus = RCC_WaitForHSEStartUp();// Wait till HSE is ready
	if(HSEStartUpStatus != ERROR){
		// Enable Prefetch Buffer
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		// HSE=25MHz, HCLK=72MHz, PCLK2=72MHz, PCLK1=36MHz 
		FLASH_SetLatency(FLASH_Latency_2);// Flash 2 wait state
		RCC_HCLKConfig(RCC_SYSCLK_Div1);	// HCLK = SYSCLK
		RCC_PCLK2Config(RCC_HCLK_Div1);	// PCLK2 = HCLK
		RCC_PCLK1Config(RCC_HCLK_Div2);	// PCLK1 = HCLK/2
		RCC_ADCCLKConfig(RCC_PCLK2_Div6);	//  ADCCLK = PCLK2/4

		// Configure PLLs 
		// PPL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz
		RCC_PREDIV2Config(RCC_PREDIV2_Div5);
		RCC_PLL2Config(RCC_PLL2Mul_8);

		RCC_PLL2Cmd(ENABLE);	// Enable PLL2
		// Wait till PLL2 is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLL2RDY) == RESET){}

		// PPL1 configuration: PLLCLK = (PLL2 / 5) * 9 = 72 MHz
		RCC_PREDIV1Config(RCC_PREDIV1_Source_PLL2, RCC_PREDIV1_Div5);
		RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);

		RCC_PLLCmd(ENABLE);	// Enable PLL
		// Wait till PLL is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET){}

		// Select PLL as system clock source
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		// Wait till PLL is used as system clock source
		while (RCC_GetSYSCLKSource() != 0x08){}
	}
}

_fx void fx_Peripheral_Configuration(void)
{
	ax_usart_t fx_usart_para;
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 
	ax_led_net_enable();
	ax_led_data_enable();
	ax_systick_init();
	fx_usart_para.usartport = 2;
	fx_usart_para.band = 3;
	fx_usart_para.datasize = 8;
	fx_usart_para.parity = 0;
	fx_usart_para.stopbit = 1;
	fx_usart_para.res = 0;
	ax_usart_init(&fx_usart_para);
}

void ax_system_init(void)
{
	fx_RCC_Configuration();
	fx_NVIC_Configuration();
	fx_Peripheral_Configuration();
}

void ax_system_running(void)
{
	for(;;){
		if((ax_systick_counter & (uint32_t)0xFF) == 0){
			ax_usart_send_string((uint8_t *)"Time has past 2.55s .\n", 24);
		}
	}
}



