#ifndef AX_USART_H
#define AX_USART_H

#include "ax_common.h"

#define AXUSART_PORT	(USART1)
#define AXUSART_IRQ		(USART1_IRQn)
#define AXUSART_IOPORT	(GPIOA)
#define AXUSART_TX_PIN	(GPIO_Pin_9)
#define AXUSART_RX_PIN	(GPIO_Pin_10)

#define AXUSART_ASSPORT	(GPIOC)
#define AXUSART_ASSPIN	(GPIO_Pin_0)

#define fx_usart_rcc()	\
		do{ \
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE); \
		    RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); \
		}while(0)

typedef struct{
	uint8_t buff[USART_RX_BUFFER_SIZE];
	volatile uint8_t data_xor;
	volatile uint16_t data_len;
}ax_usart_rcv_buff_t;


opt_result_t ax_usart_init(ax_usart_t *para);
void ax_usart_send_char(uint8_t data);
void ax_usart_send_string(uint8_t *buf, uint8_t len);
void ax_usart_send_message(char *msg);
void ax_usart_send_hex2ascii_string(uint8_t *buf, uint8_t u8len);
void ax_usart_wait_for_send_fin(void);
void ax_debug_message_output(char *msg);

#endif

