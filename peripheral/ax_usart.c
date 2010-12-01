
#include "ax_usart.h"

//static uint8_t ax_usart_buf[AXUSART_RX_BUF_CNT][AXUSART_RX_BUF_SIZE];
//static ax_usart_rcvbuf_t * ax_rcv_buf;

opt_result_t ax_usart_init(ax_usart_t *para)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_InitTypeDef USARTInitStructure;
	USART_ClockInitTypeDef USARTClockInitStructure;

	fx_usart_rcc();

	NVIC_InitStructure.NVIC_IRQChannel = AXUSART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 
											NVIC_PreemptionPriority_USART;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_SubPriority_USART2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
	// Configure USART Tx as alternate function push-pull
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = AXUSART_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(AXUSART_IOPORT, &GPIO_InitStructure);

	// Configure USART Rx as input floating
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = AXUSART_RX_PIN;
	GPIO_Init(AXUSART_IOPORT, &GPIO_InitStructure);
	
	// Configure SP3223 SHT (PE.12) as push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIO_SetBits(GPIOE, GPIO_Pin_12);

	USARTInitStructure.USART_BaudRate = 115200;
	USARTInitStructure.USART_WordLength = USART_WordLength_8b;
	USARTInitStructure.USART_StopBits = USART_StopBits_1;
	USARTInitStructure.USART_Parity = USART_Parity_No ;
	USARTInitStructure.USART_HardwareFlowControl = 
									USART_HardwareFlowControl_None;
	USARTInitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USARTClockInitStructure.USART_Clock = USART_Clock_Disable;
	USARTClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USARTClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USARTClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_DeInit(AXUSART_PORT);
	USART_ClockInit(AXUSART_PORT,&USARTClockInitStructure);
	USART_Init(AXUSART_PORT, &USARTInitStructure);

	USART_ITConfig(AXUSART_PORT, USART_IT_RXNE, ENABLE);
	USART_Cmd(AXUSART_PORT, ENABLE);
	
	return OPT_SUCCESS;
}

void ax_usart_send_char(uint8_t data)
{
	while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
	AXUSART_PORT->DR = data;
}

void ax_usart_send_string(char *buf, uint8_t len)
{
	while(len != 0){
		while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
		AXUSART_PORT->DR = *buf++;
		len --;
	}
}

/// 等待串口数据发送完成，仅特殊情况下使用
void ax_usart_wait_for_send_fin(void)
{
	while(USART_GetFlagStatus(AXUSART_PORT, USART_FLAG_TC) == RESET){;}
}



