#ifndef AX_SYSTICK_H
#define AX_SYSTICK_H

#include "ax_common.h"

#define SYSTICK_RATE_HZ    100	// 10ms
#define AX_SYSTICK_OPT_CNT	100

extern volatile uint32_t ax_systick_counter;

void ax_systick_init(void);

#endif

