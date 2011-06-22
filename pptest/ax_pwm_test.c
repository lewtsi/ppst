#include "ax_pwm_test.h"
#include "ax_usart.h"
#include "ax_rtc.h"

static uint16_t fx_pwm_duty_cycle;
static uint32_t fx_pwm_frequency;

void ax_pwm_test(void)
{
	uint8_t pwm_tmp;
	
	ax_rtc_init();
	ax_pwm_output_init();	// ***
	ax_pwm_input_init();	// ***

	while(1){
		if(ax_rtc_get_sec_flag()){
			ax_rtc_set_sec_flag(FLAG_FALSE);
			if(ax_pwm_input_get_info(&fx_pwm_duty_cycle, &fx_pwm_frequency) 
														!= OPT_RESULT_TRUE){
				ax_usart_send_message("PWM Input value error!\r\n");
			}
			pwm_tmp = fx_pwm_duty_cycle / 10;
			fx_pwm_duty_cycle %= 10;
			fx_pwm_duty_cycle |= (pwm_tmp << 4);
			ax_usart_send_hex2ascii_string((uint8_t *)&fx_pwm_frequency, 4);
			ax_usart_send_char('*');
			ax_usart_send_hex2ascii_string((uint8_t *)&fx_pwm_duty_cycle, 2);
			ax_usart_send_char('%');
			ax_usart_send_char(' ');
		}
	}
}


#endif

/* EOF */

