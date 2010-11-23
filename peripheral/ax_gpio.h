#ifndef AX_GPIO_H
#define AX_GPIO_H

#include "ax_common.h"

#define HAL_PORT_DATA_LED		( GPIOB )
#define HAL_PIN_DATA_LED		( GPIO_Pin_14 )

#define HAL_PORT_NET_LED		( GPIOB )
#define HAL_PIN_NET_LED			( GPIO_Pin_15 )

#define RCC_PORT_LED			( RCC_APB2Periph_GPIOB )

#define hal_set_net_led()		GPIO_ResetBits(HAL_PORT_NET_LED, HAL_PIN_NET_LED)
#define hal_clear_net_led()		GPIO_SetBits(HAL_PORT_NET_LED, HAL_PIN_NET_LED)

#define hal_set_data_led()		GPIO_ResetBits(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED)
#define hal_clear_data_led()	GPIO_SetBits(HAL_PORT_DATA_LED, HAL_PIN_DATA_LED)

void ax_led_net_enable(void);
void ax_led_data_enable(void);
void ax_led_net_toggle(void);
void ax_led_data_toggle(void);



#endif

