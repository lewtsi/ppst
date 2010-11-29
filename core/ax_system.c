
#include "ax_system.h"

#include "ax_gpio.h"
#include "ax_usart.h"
#include "ax_systick.h"
#include "ax_flash.h"
#include "ax_rtc.h"
#include "ax_timer.h"

static opt_result_t opt_result;
static char rd[12];

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
	uint8_t i, circle_tmp = 0x30;
	char arr[12] = "123456789ab";
	uint32_t rtc_now;

	ax_rtc_init();
	ax_timerA_init(TCNT16_MAX);
	FLASH_Unlock();
	opt_result = ax_flash_config_info_init();
	
	for(;;){
		if(ax_get_rtc_flag()){
			ax_set_rtc_flag(0);
			rtc_now = ax_get_rtc_time();
			ax_usart_send_string((uint8_t *)"RTC NOW is : ", 13);
			ax_usart_send_string((uint8_t *)&rtc_now, 4);
			ax_usart_send_char('\r');
			ax_usart_send_char('\n');
		}
		if(ax_get_timerA_flag()){
			ax_set_timerA_flag(0);
			ax_led_net_toggle();
			ax_usart_send_string((uint8_t *)"TimerA overflow!\r\n", 18);
		}
		if((ax_systick_counter & (uint32_t)0xFF) == 0){	// systick
			if(circle_tmp&0x01){
				opt_result = ax_flash_config_info_write(FLASH_DATA_TYPE_A, arr, 7);
				if(opt_result == OPT_SUCCESS){
					ax_flash_config_info_read(FLASH_DATA_TYPE_A, rd, 9);
					rd[9] = '\r';
					rd[10] = '\n';
					ax_usart_send_string((uint8_t *)rd, 11);
				}else{
					ax_usart_send_string((uint8_t *)"ERR: data A read error!\r\n", 25);
				}
				for(i=0; i<10; i++){
					arr[i] ++;
				}
			}else{
				opt_result = ax_flash_config_info_write(FLASH_DATA_TYPE_B, arr, 10);
				if(opt_result == OPT_SUCCESS){
					ax_flash_config_info_read(FLASH_DATA_TYPE_B, rd, 12);
					rd[10] = '\r';
					rd[11] = '\n';
					ax_usart_send_string((uint8_t *)rd, 12);
				}else{
					ax_usart_send_string((uint8_t *)"ERR: data B read error!\r\n", 25);
				}
				for(i=0; i<10; i++){
					arr[i] --;
				}
			}
			ax_usart_send_char(circle_tmp);
			if(++circle_tmp >= 0x3a) circle_tmp = 0x30;
			ax_usart_send_string((uint8_t *)"=Time has past 2.55s .\r\n", 24);
			while((ax_systick_counter & (uint32_t)0xFF) == 0);
		}
	}
}



