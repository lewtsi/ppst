#include "ax_systick.h"
#include "ax_gpio.h"

volatile uint32_t ax_systick_counter;
static uint8_t fx_systick_cnt_ass = AX_SYSTICK_OPT_CNT;

void ax_systick_init (void)
{
    RCC_ClocksTypeDef rcc_clocks;
    uint32_t cnts;

    RCC_GetClocksFreq(&rcc_clocks);
    cnts = (uint32_t)rcc_clocks.HCLK_Frequency / SYSTICK_RATE_HZ;
    SysTick_Config(cnts);
    NVIC_SetPriority(SysTick_IRQn, 0);	
}


void SysTick_Handler()
{
	++ax_systick_counter;
	if((-- fx_systick_cnt_ass) == 0){
		fx_systick_cnt_ass = AX_SYSTICK_OPT_CNT;
	}
}


