#include "ax_adc_test.h"
#include "ax_usart.h"

static uint16_t adc_value;

_fx void fx_adc_test_simple_ch1(void)
{
	ax_rtc_init();
	ax_adc_simple_init();
	
	while(1){
		if(ax_rtc_get_sec_flag()){
			ax_rtc_set_sec_flag(0);
			ADC_SoftwareStartConvCmd(ADC1, ENABLE);
			
			while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
			adc_value = ADC_GetConversionValue(ADC1);
			ax_usart_send_message("ADC Value : ");
			ax_usart_send_hex2ascii_string((uint8_t *)&adc_value, 2);
			ax_usart_send_message("\r\n");
		}
	}
}

_fx void fx_adc_test_ch1_dma(void)
{
	ax_adc_dma_ch1_init();
	
	while(1){
		if(DMA_GetFlagStatus(DMA1_FLAG_TC1)){
			DMA_ClearFlag(DMA1_FLAG_TC1);
			if(++ adc_value > 65500){
				adc_value = ax_adc_dma_get_coverted_value();
				ax_usart_send_message("ADC Value : ");
				ax_usart_send_hex2ascii_string((uint8_t *)&adc_value, 2);
				ax_usart_send_message("\r\n");
				adc_value = 0;
			}
		}
	}
}

_fx void fx_adc_test_analogwatchdog(void)
{
	ax_adc_analogwatchdog_init();

	while(1){
		if(ax_adc_get_awd_flag()){
			ax_adc_set_awd_flag(FLAG_FALSE);
			
			adc_value = ADC_GetConversionValue(ADC1);
			ax_usart_send_message("ADC AWD : ");
			ax_usart_send_hex2ascii_string((uint8_t *)&adc_value, 2);
			ax_usart_send_message("\r\n");
		}
	}
}


void ax_adc_test(void)
{
	//fx_adc_test_simple_ch1();
	//fx_adc_test_ch1_dma();
	fx_adc_test_analogwatchdog();
}

/* EOF */

