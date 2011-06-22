#ifndef AX_ADC_H
#define AX_ADC_H

#include "ax_common.h"

void ax_adc_simple_init(void);
void ax_adc_dma_ch1_init(void);
uint16_t ax_adc_dma_get_coverted_value(void);
void ax_adc_analogwatchdog_init(void);
void ax_adc_get_awd_flag(void);
void ax_adc_set_awd_flag(uint8_t awdflag);

#endif

/* EOF */

