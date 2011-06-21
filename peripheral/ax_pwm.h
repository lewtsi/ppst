#ifndef AX_PWM_H
#define AX_PWM_H

#include "ax_common.h"

void ax_pwm_output_init(void);
void ax_pwm_input_init(void);
opt_result_t ax_pwm_input_get_info(uint16_t *duty_cycle, uint32_t *freq);

#endif

/*EOF*/

