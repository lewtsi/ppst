#include "ax_wdg.h"

static uint8_t fx_system_boot_reason = 0;

void ax_iwdg_init(void)
{
	// IWDG timeout equal to 280 ms 
	// (the timeout may varies due to LSI frequency dispersion)
	// Enable write access to IWDG_PR and IWDG_RLR registers
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	// IWDG counter clock: 40KHz(LSI) / 256 = 156.25 Hz
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	// Set counter reload value to 360 => 2.3s
	IWDG_SetReload(360);

	// Reload IWDG counter
	IWDG_ReloadCounter();
	// Enable IWDG (the LSI oscillator will be enabled by hardware)
	IWDG_Enable();
}

opt_result_t ax_iwdg_system_boot_is_iwdg_reset(void)
{
	if((fx_system_boot_reason != AX_BOOT_IWDG) &&	// for second read
					(fx_system_boot_reason != AX_BOOT_NORMAL)){
		if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET){
			fx_system_boot_reason = AX_BOOT_IWDG;
			RCC_ClearFlag();
		}else{
			fx_system_boot_reason = AX_BOOT_NORMAL;
		}
	}
	if(fx_system_boot_reason == AX_BOOT_IWDG){
		return OPT_RESULT_TRUE;
	}else{
		return OPT_RESULT_FALSE;
	}
}

/* EOF */

