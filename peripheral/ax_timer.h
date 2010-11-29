#ifndef AX_TIMER_H
#define AX_TIMER_H

#include "ax_common.h"


#define TIMERA		TIM2
#define TIMERA_RCC	RCC_APB1Periph_TIM2
#define TIMERA_IRQ	TIM2_IRQn

typedef enum{	// @ 7.3728M/1024 @ 72M/10000
	TCNT16_2MS = 0x000E,
	TCNT16_4MS = 0x001C,
	TCNT16_5MS = 0x0024,
	TCNT16_6MS = 0x002B,
	TCNT16_7MS = 0x0032,
	TCNT16_10MS = 0x0048,
	TCNT16_20MS = 0x0090,
	TCNT16_30MS = 0x00D8,
	TCNT16_50MS = 0x0168,
	TCNT16_180MS = 0x0510,
	TCNT16_200MS = 0x05A0,
	TCNT16_250MS = 0x0708,
	TCNT16_300MS = 0x0870,
	TCNT16_500MS = 0x0E10,
	TCNT16_800MS = 0x1680,
	TCNT16_1S = 0x1C20,
	TCNT16_2S = 0x3840,
	TCNT16_5S = 0x8CA0,
	TCNT16_6S = 0xA8C0,
	TCNT16_MAX = 0xFFFF	// 9.1s
}tcnt16_t;

void ax_timerA_init(tcnt16_t TCN_vlaue);
void ax_timerA_disable(void);
void ax_timerA_enable(void);
uint8_t ax_get_timerA_flag(void);
void ax_set_timerA_flag(uint8_t tflag);
uint16_t ax_get_timerA_counter(void);
void ax_set_timerA_counter(uint16_t cnt);

#endif

