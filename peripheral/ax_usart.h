#ifndef AX_USART_H
#define AX_USART_H

#include "ax_common.h"

#define AXUSART_RX_BUF_CNT		((uint8_t) 2)
#define AXUSART_RX_BUF_SIZE		((uint8_t)128)

#define AXUSART_PORT	(USART2)
#define AXUSART_IRQ		(USART2_IRQn)
#define AXUSART_IOPORT	(GPIOD)
#define AXUSART_TX_PIN	(GPIO_Pin_5)
#define AXUSART_RX_PIN	(GPIO_Pin_6)

#define fx_usart_rcc()	\
		do{ \
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | \
				RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE); \
		    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); \
		}while(0)

typedef struct{
	uint8_t usartport;
	uint8_t band;
	uint8_t datasize;
	uint8_t parity;
	uint8_t stopbit;
	void * res;
}ax_usart_t;

typedef struct{
	uint8_t *buff;
	uint8_t data_xor;
	uint8_t data_len;
	uint8_t attribute;	// µÍËÄÎ»CNT
}ax_usart_rcvbuf_t;


opt_result_t ax_usart_init(ax_usart_t *para);
void ax_usart_send_char(uint8_t data);
void ax_usart_send_string(uint8_t *buf, uint8_t len);
void ax_usart_send_char_message(char *buf);
void ax_usart_wait_for_send_fin(void);
void ax_debug_message_output(char *msg);

#endif

