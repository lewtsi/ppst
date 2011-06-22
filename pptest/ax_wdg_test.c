#include "ax_wdg_test.h"
#include "ax_usart.h"
#include "ax_rtc.h"

void ax_iwdg_test(void)
{
	uint8_t iwdg_ass_cnt = 0;
	
	if(ax_iwdg_system_boot_is_iwdg_reset()){	// ***
		ax_usart_send_message("IWDG RESET!\r\n");
	}else{
		ax_usart_send_message("PWR ON!\r\n");
	}
	ax_rtc_init();
	
	ax_iwdg_init();		// ***

	while(1){
		if(ax_rtc_get_sec_flag()){
			ax_rtc_set_sec_flag(FLAG_FALSE);
			if(++ iwdg_ass_cnt < 10){
				ax_iwdg_reset();	// ***
				ax_usart_send_message("IWDG counter : ");
				ax_usart_send_char(iwdg_ass_cnt + '0');
				ax_usart_send_message("\r\n");				
			}else{
				ax_usart_send_message("IWDG overflow : ");
				ax_usart_send_char(iwdg_ass_cnt - 9 + '0');
				ax_usart_send_message("\r\n");			
			}
		}
	}
}

/* EOF */

