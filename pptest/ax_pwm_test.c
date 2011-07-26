#include "ax_pwm_test.h"
#include "ax_usart.h"
#include "ax_rtc.h"

void ax_pwm_test(void)
{
	uint16_t *duty_cycle;
	uint32_t *freq;
	
	ax_rtc_init();
	ax_pwm_output_init();
	ax_pwm_input_multi_channel_init();

	while(1){
		if(ax_rtc_get_sec_flag()){
			ax_rtc_set_sec_flag(FLAG_FALSE);
			
			ax_usart_send_message("------------------------------\r\n");
			if(ax_pwm_input_get_info(&duty_cycle, &freq) != 0){
				ax_usart_send_message("PWM Input value error!\r\n");
			}
			ax_usart_send_hex2ascii_string((uint8_t *)duty_cycle, 8);
			ax_usart_send_message("\r\n");
			ax_usart_send_hex2ascii_string((uint8_t *)freq, 16);
			ax_usart_send_message("\r\n");
			ax_pwm_output_config(*duty_cycle);
		}
	}
}


#endif

/* EOF */

