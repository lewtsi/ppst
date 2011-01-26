
#include "ax_gpio.h"

void ax_led_net_enable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_PORT_LED | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = HAL_PIN_NET_LED;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(HAL_PORT_NET_LED, &GPIO_InitStructure);
}

void ax_led_data_enable(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	// Enable the GPIO_LED Clock & disable JTAG function
	RCC_APB2PeriphClockCmd(RCC_PORT_LED | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); 

	GPIO_InitStructure.GPIO_Pin = HAL_PIN_DATA_LED;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(HAL_PORT_DATA_LED, &GPIO_InitStructure);
}

void ax_led_net_toggle(void)
{
	if(GPIO_ReadOutputDataBit(HAL_PORT_NET_LED, HAL_PIN_NET_LED)){
		GPIO_ResetBits(HAL_PORT_NET_LED, HAL_PIN_NET_LED);
	}else{
		GPIO_SetBits(HAL_PORT_NET_LED, HAL_PIN_NET_LED);
	}
}

void ax_led_data_toggle(void)
{
	if(GPIO_ReadOutputDataBit(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED)){
		GPIO_ResetBits(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED);
	}else{
		GPIO_SetBits(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED);
	}
}


